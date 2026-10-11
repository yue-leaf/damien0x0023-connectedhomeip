# TI LCD Mono

The generated `../src/StatusFontData.h` is a monochrome subset derived from
Microsoft Cascadia Mono, licensed under the SIL Open Font License 1.1 in
`OFL.txt`. The derivative is named TI LCD Mono. All generated glyphs retain
that license; the renderer and generator code remain Apache-2.0.

The body, compact, and large glyphs are rasterized independently at their
native sizes. Firmware does not load a TrueType font or rescale the glyphs.
Only printable ASCII is included; unsupported characters render as `?`.

To regenerate using Pillow and the Cascadia Mono variable font:

```sh
python3 tools/generate_status_font.py --font /path/to/CascadiaMono.ttf
```

The generated header records the source font checksum. No generation step or
additional dependency is needed in the firmware workflow.
