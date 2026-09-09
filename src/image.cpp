#include "image.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <PNGdec.h>
#include "config.h"

// Zeilenpuffer-Grenze: der GC9A01 ist 240 px breit, etwas Reserve fuer
// leicht zu grosse Vorlagen. Breitere PNGs weist imageShowPng() ab.
#define IMAGE_MAX_WIDTH 320
#define IMAGE_MAX_FILES 16
#define IMAGE_NAME_MAX  48

static PNG  s_png;      // ~40 KB im BSS - fuer den RP2350 (520 KB RAM) unkritisch
static File s_file;

static Adafruit_GC9A01A *s_tft     = nullptr;
static int16_t           s_originX = 0;
static int16_t           s_originY = 0;
static uint16_t          s_bg      = 0x0000;
static uint16_t          s_line[IMAGE_MAX_WIDTH];

static char   s_names[IMAGE_MAX_FILES][IMAGE_NAME_MAX];
static size_t s_count = 0;

static uint8_t s_brightness = BRIGHTNESS_DEFAULT;   // 0..255, Software-Dimmung

// Vollbild-Framebuffer (ungedimmt). Damit laesst sich die Helligkeit ohne
// erneutes PNG-Dekodieren anpassen - imageRefresh() schiebt den Puffer nur
// noch (gedimmt) auf das Display. ~115 KB im BSS, unkritisch fuer den RP2350.
static uint16_t s_fb[TFT_WIDTH * TFT_HEIGHT];
static bool     s_fbValid = false;

// RGB565-Pixel linear abdunkeln (jeder Kanal * level / 255).
static inline uint16_t dim565(uint16_t c)
{
    const uint8_t b = s_brightness;
    if (b >= 255)
        return c;
    uint16_t r = ((c >> 11) & 0x1F) * b / 255;
    uint16_t g = ((c >> 5)  & 0x3F) * b / 255;
    uint16_t l = ( c        & 0x1F) * b / 255;
    return (uint16_t)((r << 11) | (g << 5) | l);
}

// Framebuffer mit der aktuellen Helligkeit auf das Display schieben.
static void blitFb()
{
    if (!s_tft)
        return;

    s_tft->startWrite();
    s_tft->setAddrWindow(0, 0, TFT_WIDTH, TFT_HEIGHT);
    if (s_brightness >= 255)
    {
        s_tft->writePixels(s_fb, (uint32_t)TFT_WIDTH * TFT_HEIGHT);
    }
    else
    {
        for (int16_t row = 0; row < TFT_HEIGHT; row++)
        {
            const uint16_t *src = &s_fb[row * TFT_WIDTH];
            for (int16_t i = 0; i < TFT_WIDTH; i++)
                s_line[i] = dim565(src[i]);
            s_tft->writePixels(s_line, TFT_WIDTH);
        }
    }
    s_tft->endWrite();
}

// --------------------------------------------------------------------------
// PNGdec <-> LittleFS: Datei-Callbacks
// --------------------------------------------------------------------------
static void *pngOpen(const char *filename, int32_t *size)
{
    s_file = LittleFS.open(filename, "r");
    if (!s_file)
        return nullptr;
    *size = s_file.size();
    return &s_file;
}

static void pngClose(void *)
{
    if (s_file)
        s_file.close();
}

static int32_t pngRead(PNGFILE *, uint8_t *buf, int32_t len)
{
    if (!s_file)
        return 0;
    return s_file.read(buf, len);
}

static int32_t pngSeek(PNGFILE *, int32_t pos)
{
    if (!s_file)
        return 0;
    return s_file.seek(pos) ? pos : 0;
}

// --------------------------------------------------------------------------
// Decode-Callback: PNGdec liefert eine fertig dekodierte Pixelzeile
// --------------------------------------------------------------------------
static int pngDraw(PNGDRAW *pDraw)
{
    int w = pDraw->iWidth;
    if (w > IMAGE_MAX_WIDTH)
        w = IMAGE_MAX_WIDTH;

    // RGB565, teiltransparente Pixel gegen s_bg verrechnet.
    s_png.getLineAsRGB565(pDraw, s_line, PNG_RGB565_LITTLE_ENDIAN, s_bg);

    const int16_t y = s_originY + pDraw->y;
    if (y < 0 || y >= TFT_HEIGHT)
        return 1;                       // Zeile liegt ausserhalb des Displays

    int16_t   x = s_originX;
    uint16_t *p = s_line;
    if (x < 0) { p -= x; w += x; x = 0; }           // links abschneiden
    if (x + w > TFT_WIDTH) w = TFT_WIDTH - x;       // rechts abschneiden
    if (w <= 0)
        return 1;

    // Ungedimmt in den Framebuffer; die Ausgabe erfolgt gesammelt in blitFb().
    memcpy(&s_fb[y * TFT_WIDTH + x], p, (size_t)w * sizeof(uint16_t));
    return 1;
}

// --------------------------------------------------------------------------
// oeffentliche API
// --------------------------------------------------------------------------
static void scanPngs()
{
    s_count = 0;
    Dir dir = LittleFS.openDir("/");
    while (dir.next() && s_count < IMAGE_MAX_FILES)
    {
        String name = dir.fileName();
        if (!name.startsWith("/"))
            name = "/" + name;

        String lower = name;
        lower.toLowerCase();
        if (!lower.endsWith(".png"))
            continue;

        snprintf(s_names[s_count], IMAGE_NAME_MAX, "%s", name.c_str());
        s_count++;
    }
}

bool imageBegin()
{
    if (!LittleFS.begin())
    {
        Serial.println(F("[image] LittleFS-Mount fehlgeschlagen - "
                         "'pio run -e pico2 -t uploadfs' ausgefuehrt?"));
        return false;
    }

    scanPngs();
    Serial.printf("[image] LittleFS ok, %u PNG(s) gefunden\n", (unsigned)s_count);
    for (size_t i = 0; i < s_count; i++)
        Serial.printf("        %s\n", s_names[i]);
    return true;
}

size_t imageCount()
{
    return s_count;
}

const char *imageName(size_t i)
{
    return i < s_count ? s_names[i] : "";
}

void imageSetBrightness(uint8_t level)
{
    s_brightness = level;
}

uint8_t imageGetBrightness()
{
    return s_brightness;
}

void imageRefresh()
{
    if (s_fbValid)
        blitFb();
}

bool imageShowPng(Adafruit_GC9A01A &tft, const char *path, uint16_t bg)
{
    s_tft = &tft;
    s_bg  = bg;

    int rc = s_png.open(path, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    if (rc != PNG_SUCCESS)
    {
        Serial.printf("[image] open '%s' fehlgeschlagen (rc=%d)\n", path, rc);
        return false;
    }

    const int w = s_png.getWidth();
    const int h = s_png.getHeight();
    Serial.printf("[image] %s  %dx%d  bpp=%d  pixeltype=%d\n",
                  path, w, h, s_png.getBpp(), s_png.getPixelType());

    if (w > IMAGE_MAX_WIDTH)
    {
        Serial.printf("[image] %d px zu breit (max %d) - Vorlage verkleinern\n",
                      w, IMAGE_MAX_WIDTH);
        s_png.close();
        return false;
    }

    // Zentrieren - funktioniert unveraendert auch bei abweichender Bildgroesse.
    s_originX = (int16_t)((TFT_WIDTH  - w) / 2);
    s_originY = (int16_t)((TFT_HEIGHT - h) / 2);

    // Framebuffer mit der Hintergrundfarbe fuellen (Rand um das Bild).
    for (uint32_t i = 0; i < (uint32_t)TFT_WIDTH * TFT_HEIGHT; i++)
        s_fb[i] = bg;

    rc = s_png.decode(nullptr, 0);
    s_png.close();

    if (rc != PNG_SUCCESS)
    {
        Serial.printf("[image] decode '%s' fehlgeschlagen (rc=%d)\n", path, rc);
        s_fbValid = false;
        return false;
    }

    s_fbValid = true;
    blitFb();
    return true;
}
