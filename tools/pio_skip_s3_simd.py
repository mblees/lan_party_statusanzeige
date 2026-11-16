# PlatformIO pre-script (siehe extra_scripts in platformio.ini).
#
# PNGdec bringt mit s3_simd_rgb565.S eine SIMD-Routine nur fuer den ESP32-S3
# mit. Sie wird auf jedem ESP32 assembliert und braucht dafuer den esp-dsp-Header
# dsps_fft2r_platform.h, den Arduino-ESP32 3.x fuer den ESP32-C3 nicht mitliefert
# -> Build-Abbruch. Aufgerufen wird sie nur bei ARDUINO_ESP32S3_DEV (png.inl),
# auf allen anderen Chips kann die Datei also gefahrlos aus dem Build fallen.
Import("env")

if env.BoardConfig().get("build.mcu", "") != "esp32s3":
    env.AddBuildMiddleware(lambda node: None, "*s3_simd_rgb565.S")
