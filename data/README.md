# data/ – LittleFS contents

Everything in this folder is written to the LittleFS partition of the ESP32-C3
flash with

```
pio run -t uploadfs
```

It is separate from the firmware, so status images can be swapped without
recompiling.

## Images

- PNG, ideally 240×240 px (round display, visible circle ≈ 234 px).
- 8-bit RGB or RGBA; palette and grayscale work as well.
- Transparency (alpha) is blended against the `bg` colour passed to `imageShowPng()`.
- Up to 16 images; the path including the leading `/` must stay below 48 characters.
- Prepare source images with the helper script (zooms in until the round
  display is filled edge to edge; overflow is cropped centred):

  ```
  python tools/prepare_image.py raw.png smoking
  ```

  writes a display-ready `data/smoking.png`. `--fit` fits the whole image and
  keeps an intentional margin (e.g. a Visio export), `--no-trim` keeps the
  transparent border in cover mode as well, `--no-circle` leaves it square.

After the boot logo the firmware shows the first `*.png` from the root
directory; the image touch button cycles through the rest.
