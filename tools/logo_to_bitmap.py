#!/usr/bin/env python3
"""Convert a black/white logo image into a 1-bit bitmap array for intro.cpp.

Usage:
    python tools/logo_to_bitmap.py logo_binary.png [max_w max_h]

Defaults to 208x208 (fits inside the 240px round GC9A01 with margin).
White pixels -> lit pixels, drawn later in the intro colour.
Requires Pillow ( pip install Pillow ).
"""
import sys
from PIL import Image, ImageFilter

DILATE = 3
THRESHOLD = 100


def main(path, max_w, max_h):
    src = Image.open(path).convert("L")
    bbox = src.point(lambda v: 255 if v > 60 else 0).getbbox()
    if bbox:
        src = src.crop(bbox)

    sw, sh = src.size
    scale = min(max_w / sw, max_h / sh)
    nw, nh = max(1, round(sw * scale)), max(1, round(sh * scale))
    # pad to a whole byte in width so drawBitmap math is simple
    W = (nw + 7) // 8 * 8
    H = nh

    small = (src.filter(ImageFilter.MaxFilter(DILATE))
                .resize((nw * 3, nh * 3), Image.LANCZOS)
                .resize((nw, nh), Image.LANCZOS))
    canvas = Image.new("L", (W, H), 0)
    canvas.paste(small, ((W - nw) // 2, 0))
    bw = canvas.point(lambda v: 255 if v > THRESHOLD else 0, mode="1")
    bw.resize((W * 3, H * 3), Image.NEAREST).save("logo_preview.png")

    px = bw.load()
    rowbytes = W // 8
    out = []
    for y in range(H):
        for bx in range(rowbytes):
            byte = 0
            for bit in range(8):
                if px[bx * 8 + bit, y]:
                    byte |= 1 << (7 - bit)
            out.append(byte)

    print(f"// {path} -> {W}x{H}, {len(out)} bytes")
    print(f"#define INTRO_LOGO_WIDTH  {W}")
    print(f"#define INTRO_LOGO_HEIGHT {H}")
    print(f"const unsigned char INTRO_LOGO_BMP[{len(out)}] PROGMEM = {{")
    for r in range(H):
        row = out[r * rowbytes:(r + 1) * rowbytes]
        print("    " + "".join(f"0x{v:02X}, " for v in row).rstrip())
    print("};")


if __name__ == "__main__":
    if len(sys.argv) not in (2, 4):
        sys.exit(__doc__)
    mw = int(sys.argv[2]) if len(sys.argv) == 4 else 208
    mh = int(sys.argv[3]) if len(sys.argv) == 4 else 208
    main(sys.argv[1], mw, mh)
