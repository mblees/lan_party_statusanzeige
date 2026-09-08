#include "circletext.h"
#include <Arduino.h>
#include <string.h>
#include <math.h>
#include "config.h"

// ---------------------------------------------------------------------------
// Klassischer Adafruit-GFX-Font bei Groesse 1: Zellenraster 6x8 px, davon
// 5x7 px sichtbare "Tinte" (oben links in der Zelle), 1 px Abstand rechts und
// unten. Bei Groesse s skaliert alles linear mit s.
// ---------------------------------------------------------------------------
static const int   S        = TFT_TEXT_SIZE;
static const float ADVANCE  = 6.0f * S;   // Vorschub pro Zeichen (x)
static const float INK_H    = 7.0f * S;   // sichtbare Zeichenhoehe
static const float LINE_H   = 8.0f * S;   // Zeilenabstand (y)

static const float R   = (float)TFT_TEXT_RADIUS;
static const float CX  = (TFT_WIDTH  - 1) / 2.0f;   // Kreismittelpunkt
static const float CY  = (TFT_HEIGHT - 1) / 2.0f;

// Obergrenze fuer die Zeilenzahl (Groesse 1 -> ca. 29 Zeilen passen in den Kreis).
#define MAX_LINES 32

// Sichtbare Breite von n Zeichen in Pixeln (letztes Zeichen ohne Abstand rechts).
static float inkWidth(int n)
{
    return n > 0 ? (ADVANCE * n - S) : 0.0f;
}

// Wie viele Zeichen passen in eine Zeile, deren Tinte vertikal von yTop bis
// yTop+INK_H reicht, ohne dass ein Zeichen den Kreisrand schneidet?
// Die Zeile ist horizontal zentriert; die kritische Ecke ist die vom
// Mittelpunkt vertikal am weitesten entfernte Zeilenkante.
static int lineCapacity(float yTop)
{
    float dy = fmaxf(fabsf(yTop - CY), fabsf(yTop + INK_H - CY));
    float inside = R * R - dy * dy;
    if (inside <= 0.0f)
        return 0;
    float half = sqrtf(inside);                  // halbe nutzbare Breite
    // inkWidth(n)/2 <= half  ->  (ADVANCE*n - S)/2 <= half
    int n = (int)floorf((2.0f * half + S) / ADVANCE);
    return n > 0 ? n : 0;
}

// y-Position (obere Tintenkante) der ersten von k vertikal zentrierten Zeilen.
static float firstLineTop(int k)
{
    float blockH = (k - 1) * LINE_H + INK_H;
    return CY - blockH / 2.0f;
}

// Bricht text greedy an Wortgrenzen in hoechstens k Zeilen um; die Kapazitaet
// jeder Zeile steht in cap[]. Das Ergebnis (Zeilen durch '\n' getrennt) landet
// in out. Ist truncate == false, liefert die Funktion false, sobald der Text
// nicht in k Zeilen passt. Bei truncate == true wird ueberschuessiger Text
// verworfen und true zurueckgegeben.
static bool wrapInto(const char *text, const int *cap, int k,
                     char *out, size_t outCap, bool truncate)
{
    size_t o = 0;
    size_t i = 0;
    size_t n = strlen(text);
    int    line    = 0;
    int    lineLen = 0;          // logische Zeichen in der aktuellen Zeile
    bool   fit     = true;

    while (i < n)
    {
        while (i < n && text[i] == ' ')          // fuehrende Leerzeichen
            i++;
        if (i >= n)
            break;

        size_t ws = i;                           // Wortanfang
        while (i < n && text[i] != ' ')
            i++;
        int wl  = (int)(i - ws);                 // Wortlaenge
        int off = 0;                             // bereits platzierter Teil

        while (off < wl)
        {
            int space = (lineLen > 0) ? 1 : 0;   // Trennzeichen zum Vorwort
            int room  = cap[line] - lineLen - space;
            int take;

            if (room >= wl - off)
                take = wl - off;                 // Rest des Wortes passt
            else if (lineLen == 0)
                take = cap[line];                // Wort laenger als ganze Zeile
            else
                take = 0;                        // erst Zeilenumbruch, dann neu

            if (take <= 0)
            {
                line++;
                if (line >= k)
                {
                    if (truncate)
                        goto done;
                    return false;
                }
                if (o + 1 < outCap)
                    out[o++] = '\n';
                lineLen = 0;
                continue;
            }

            if (space && o + 1 < outCap)
                out[o++] = ' ';
            for (int t = 0; t < take && o + 1 < outCap; t++)
                out[o++] = text[ws + off + t];
            lineLen += take + space;
            off     += take;
        }
    }

done:
    out[o < outCap ? o : outCap - 1] = '\0';
    return fit;
}

static int countLines(const char *s)
{
    if (s == nullptr || *s == '\0')
        return 0;
    int n = 1;
    for (; *s; s++)
        if (*s == '\n')
            n++;
    return n;
}

// Berechnet die Zeilenkapazitaeten fuer k zentrierte Zeilen. Gibt false zurueck,
// wenn eine Zeile nicht mehr vollstaendig in den Kreis passt.
static bool capacities(int k, int *cap)
{
    float top = firstLineTop(k);
    for (int i = 0; i < k; i++)
    {
        cap[i] = lineCapacity(top + i * LINE_H);
        if (cap[i] <= 0)
            return false;
    }
    return true;
}

// Sucht die kleinste Zeilenzahl, in die der Text passt (schoenste vertikale
// Zentrierung), und schreibt den umgebrochenen Text nach out. Passt der Text
// nirgends hinein, wird er mit der groesstmoeglichen Zeilenzahl abgeschnitten.
static void layout(const char *text, char *out, size_t outCap)
{
    int cap[MAX_LINES];
    int lastValidK = 0;

    for (int k = 1; k <= MAX_LINES; k++)
    {
        if (!capacities(k, cap))
            break;                               // groesser wird nur schlechter
        lastValidK = k;

        if (wrapInto(text, cap, k, out, outCap, false))
            return;
    }

    if (lastValidK > 0)
    {
        capacities(lastValidK, cap);
        wrapInto(text, cap, lastValidK, out, outCap, true);
    }
    else
    {
        out[0] = '\0';
    }
}

void circleTextShow(Adafruit_GC9A01A &tft, const char *text)
{
    tft.fillScreen(TFT_TEXT_BG);
    if (text == nullptr || text[0] == '\0')
        return;

    char wrapped[SERIAL_LINE_MAX + MAX_LINES + 1];
    layout(text, wrapped, sizeof(wrapped));

    int lines = countLines(wrapped);
    if (lines == 0)
        return;

    tft.setTextSize(S);
    tft.setTextColor(TFT_TEXT_FG);
    tft.setTextWrap(false);

    float y = firstLineTop(lines);
    const char *p = wrapped;

    while (*p)
    {
        const char *e = p;
        while (*e && *e != '\n')
            e++;
        int chars = (int)(e - p);

        int16_t x = (int16_t)lroundf(CX - inkWidth(chars) / 2.0f);
        tft.setCursor(x, (int16_t)lroundf(y));
        for (const char *c = p; c < e; c++)
            tft.write((uint8_t)*c);

        y += LINE_H;
        p  = (*e == '\n') ? e + 1 : e;
    }
}
