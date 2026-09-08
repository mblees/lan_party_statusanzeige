#!/usr/bin/env python3
"""Beliebiges Bild -> displayfertiges PNG in data/ (fuer LittleFS / GC9A01).

Usage:
    python tools/prepare_image.py <bild> [name] [groesse] [optionen]

    python tools/prepare_image.py roh.png                # -> data/roh.png
    python tools/prepare_image.py foto.jpg ente          # -> data/ente.png
    python tools/prepare_image.py logo.png logo 200      # Zielflaeche 200 px
    python tools/prepare_image.py icon.png --fit          # ganzes Bild einpassen
    python tools/prepare_image.py visio.png --fit --no-trim  # Rand 1:1 behalten
    python tools/prepare_image.py bild.png --no-circle    # ohne Kreismaske

Ablauf:
1. transparente Raender wegschneiden   (aus, sobald --fit oder --no-trim)
2. skalieren:
   - Standard "Cover": so weit hineinzoomen, dass die runde Anzeige randlos
     gefuellt ist; Ueberstand wird zentriert abgeschnitten.
   - --fit: ganzes Bild proportional einpassen (nichts geht verloren, evtl.
     bleibt aussen Rand).
3. weiche Kreismaske (sauberer Rand am runden Display), --no-circle laesst es eckig
4. als RGBA-PNG nach data/ schreiben  (dort holt es 'pio run -t uploadfs')

Optionen:
    --fit         einpassen statt zoomen. Deaktiviert zugleich das automatische
                  Wegschneiden transparenter Raender (--trim erzwingt es wieder).
    --no-trim     transparente Raender NICHT wegschneiden (fuer Vorlagen, in
                  denen der Abstand bewusst gesetzt ist, z. B. Visio-Export).
    --trim        Raender wegschneiden, auch bei --fit.
    --no-circle   keine Kreismaske.

Auf dem Pico wird nichts mehr skaliert - das Bild kommt pixelgenau aufs Display.

Requires Pillow ( pip install Pillow ).
"""
import os
import sys

from PIL import Image, ImageChops, ImageDraw

DEFAULT_SIZE = 240
KNOWN_OPTS = {"--fit", "--trim", "--no-trim", "--no-circle"}


def resize_cover(im, size):
    """Bild so skalieren, dass es size x size vollstaendig ueberdeckt, dann
    zentriert auf size x size zuschneiden (kein Rand, Ueberstand faellt weg)."""
    scale = max(size / im.width, size / im.height)
    nw, nh = max(size, round(im.width * scale)), max(size, round(im.height * scale))
    im = im.resize((nw, nh), Image.LANCZOS)
    left, top = (nw - size) // 2, (nh - size) // 2
    return im.crop((left, top, left + size, top + size))


def resize_fit(im, size):
    """Bild proportional in size x size einpassen (ganzes Bild bleibt sichtbar)."""
    im = im.copy()
    im.thumbnail((size, size), Image.LANCZOS)
    return im


def circle_mask(d):
    """Weiche, 4x oversamplete Kreismaske der Kantenlaenge d."""
    mask = Image.new("L", (d * 4, d * 4), 0)
    ImageDraw.Draw(mask).ellipse((0, 0, d * 4 - 1, d * 4 - 1), fill=255)
    return mask.resize((d, d), Image.LANCZOS)


def main(argv):
    opts = {a for a in argv if a.startswith("--")}
    pos = [a for a in argv if not a.startswith("--")]
    unknown = opts - KNOWN_OPTS
    if not pos or unknown:
        sys.exit(("Unbekannte Option: " + ", ".join(sorted(unknown)) + "\n\n"
                  if unknown else "") + __doc__)

    src = pos[0]
    name = pos[1] if len(pos) > 1 else os.path.splitext(os.path.basename(src))[0]
    size = int(pos[2]) if len(pos) > 2 else DEFAULT_SIZE
    cover = "--fit" not in opts
    circle = "--no-circle" not in opts
    # Raender wegschneiden: bei Cover Standard an, bei --fit Standard aus.
    # --trim / --no-trim ueberschreiben das explizit.
    if "--no-trim" in opts:
        trim = False
    elif "--trim" in opts:
        trim = True
    else:
        trim = cover

    im = Image.open(src).convert("RGBA")
    if trim:
        bbox = im.getbbox()
        if bbox:
            im = im.crop(bbox)

    im = resize_cover(im, size) if cover else resize_fit(im, size)

    # Quadratische Zeichenflaeche; bei --fit wird das Bild darin zentriert.
    d = size if cover else max(im.size)
    canvas = Image.new("RGBA", (d, d), (0, 0, 0, 0))
    canvas.paste(im, ((d - im.width) // 2, (d - im.height) // 2), im)

    if circle:
        canvas.putalpha(ImageChops.multiply(canvas.getchannel("A"), circle_mask(d)))

    im = canvas
    os.makedirs("data", exist_ok=True)
    out = os.path.join("data", name + ".png")
    im.save(out, optimize=True)
    print(f"{src} -> {out}  ({im.width}x{im.height}, "
          f"{'cover/zoom' if cover else 'fit'}"
          f"{', trim' if trim else ', kein trim'}"
          f"{', Kreis' if circle else ''})")


if __name__ == "__main__":
    main(sys.argv[1:])
