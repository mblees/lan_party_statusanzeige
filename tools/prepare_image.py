#!/usr/bin/env python3
"""Beliebiges Bild -> displayfertiges PNG in data/ (fuer LittleFS / GC9A01).

Usage:
    python tools/prepare_image.py <bild> [name] [groesse] [--no-circle]

    python tools/prepare_image.py roh.png                # -> data/roh.png
    python tools/prepare_image.py foto.jpg smoking       # -> data/smoking.png
    python tools/prepare_image.py logo.png logo 200      # max. 200 px
    python tools/prepare_image.py bild.png --no-circle   # ohne Kreismaske

Was passiert:
- proportional auf hoechstens GROESSE x GROESSE verkleinern (Default 240)
- vollstaendig transparente Raender wegschneiden
- weiche Kreismaske anlegen (passt zum runden Display), mit --no-circle aus
- als RGBA-PNG nach data/ schreiben  (dort holt es 'pio run -t uploadfs')

Damit lassen sich Vorlagen beliebiger Aufloesung/Groesse einheitlich auf das
240er-Display bringen; auf dem Pico wird nichts mehr skaliert.

Requires Pillow ( pip install Pillow ).
"""
import os
import sys

from PIL import Image, ImageChops, ImageDraw

DEFAULT_SIZE = 240


def main(argv):
    opts = {a for a in argv if a.startswith("--")}
    pos = [a for a in argv if not a.startswith("--")]
    if not pos:
        sys.exit(__doc__)

    src = pos[0]
    name = pos[1] if len(pos) > 1 else os.path.splitext(os.path.basename(src))[0]
    size = int(pos[2]) if len(pos) > 2 else DEFAULT_SIZE
    circle = "--no-circle" not in opts

    im = Image.open(src).convert("RGBA")
    bbox = im.getbbox()
    if bbox:
        im = im.crop(bbox)
    im.thumbnail((size, size), Image.LANCZOS)

    if circle:
        d = max(im.size)  # quadratische Zeichenflaeche, Bild wird einbeschrieben
        # 4x oversampled Kreis -> weiche Kante nach dem Verkleinern
        mask = Image.new("L", (d * 4, d * 4), 0)
        ImageDraw.Draw(mask).ellipse((0, 0, d * 4 - 1, d * 4 - 1), fill=255)
        mask = mask.resize((d, d), Image.LANCZOS)

        canvas = Image.new("RGBA", (d, d), (0, 0, 0, 0))
        canvas.paste(im, ((d - im.width) // 2, (d - im.height) // 2), im)
        canvas.putalpha(ImageChops.multiply(canvas.getchannel("A"), mask))
        im = canvas

    os.makedirs("data", exist_ok=True)
    out = os.path.join("data", name + ".png")
    im.save(out, optimize=True)
    print(f"{src} -> {out}  ({im.width}x{im.height})")


if __name__ == "__main__":
    main(sys.argv[1:])
