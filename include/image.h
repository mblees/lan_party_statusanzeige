#ifndef IMAGE_H
#define IMAGE_H

#include <Adafruit_GC9A01A.h>

// PNG-Bilder aus dem LittleFS-Dateisystem des Pico 2 anzeigen.
//
// Bilder liegen im Projektordner unter data/ und werden getrennt von der
// Firmware ins Flash geschrieben:
//
//     pio run -e pico2 -t uploadfs      (Dateisystem flashen)
//     pio run -e pico2 -t upload        (Firmware flashen)
//
// So lassen sich Grafiken austauschen, ohne neu zu kompilieren.

// LittleFS einbinden und das Wurzelverzeichnis nach *.png durchsuchen.
// MUSS vor dem ersten imageShowPng() und vor bootselResetBegin() laufen:
// scheitert das Mounten, formatiert LittleFS das Flash - ein Schreibzugriff,
// der nicht mit dem BOOTSEL-Reset-Timer kollidieren darf.
// Liefert false, wenn kein Dateisystem verfuegbar ist.
bool imageBegin();

// PNG aus dem Dateisystem zentriert auf dem Display anzeigen.
//   path : z. B. "/smoking.png"
//   bg   : Flaechen-/Transparenzfarbe (RGB565). Der Bildschirm wird damit
//          geloescht; teiltransparente Pixel werden damit verrechnet.
// Bilder duerfen kleiner oder groesser als 240x240 sein - sie werden zentriert
// und (bei Ueberbreite/-hoehe) am Displayrand abgeschnitten. Nicht skaliert;
// dafuer die Vorlage vorher mit tools/prepare_image.py aufbereiten.
// Liefert false bei Fehler (Datei fehlt, kein gueltiges PNG, zu breit).
bool imageShowPng(Adafruit_GC9A01A &tft, const char *path, uint16_t bg = 0x0000);

// --- Fuer den Demo-Modus: von imageBegin() eingelesene Liste aller *.png
//     im Wurzelverzeichnis des Dateisystems. ---
size_t      imageCount();
const char *imageName(size_t i);   // vollstaendiger Pfad, z. B. "/smoking.png"

// Software-Helligkeit: skaliert beim Zeichnen jeden Pixel (0 = schwarz,
// 255 = unveraendert). Das Display-Modul hat keinen Backlight-Pin, deshalb
// wird die Helligkeit ueber die Pixelwerte nachgebildet. Wirkt ab dem
// naechsten imageShowPng(); den aktuellen Inhalt ggf. neu zeichnen.
void    imageSetBrightness(uint8_t level);
uint8_t imageGetBrightness();

// Zuletzt gezeigtes Bild mit der aktuellen Helligkeit erneut ausgeben - ohne
// PNG-Dekodierung, nur ein (gedimmter) Kopiervorgang aus dem Framebuffer.
// Schnell genug fuer Live-Aenderungen am Helligkeitsregler. Ohne vorheriges
// erfolgreiches imageShowPng() passiert nichts.
void    imageRefresh();

#endif // IMAGE_H
