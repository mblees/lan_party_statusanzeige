# data/ – Inhalt des LittleFS-Dateisystems

Alles in diesem Ordner landet mit

```
pio run -t uploadfs
```

im LittleFS-Bereich des ESP32-C3-Flash (Partitionstabelle der Board-Definition). Getrennt von der Firmware – Bilder
tauschen geht also ohne Neucompilieren.

## Bilder

- PNG, moeglichst <= 240x240 px (rundes Display, sichtbarer Kreis 234 px).
- 8-bit RGB oder RGBA; Palette/Graustufen gehen auch.
- Transparenz (Alpha) wird gegen die `bg`-Farbe aus `imageShowPng()` verrechnet.
- Vorlagen aufbereiten (zoomt automatisch, bis die runde Anzeige randlos
  gefuellt ist; Ueberstand wird zentriert abgeschnitten):

  ```
  python tools/prepare_image.py rohbild.png smoking
  ```

  schreibt `data/smoking.png` passend zurecht. `--fit` passt das ganze Bild ein
  und behaelt einen bewusst gesetzten Rand (Visio-Export), `--no-trim` behaelt
  den transparenten Rand auch im Cover-Modus, `--no-circle` laesst es eckig.

Die Demo in `src/main.cpp` zeigt beim Start automatisch jede `*.png` aus dem
Wurzelverzeichnis nacheinander an.
