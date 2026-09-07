# Player craft in 3D — turntable

An original low-poly interpretation of the S.T.U.N. Runner tunnel
speeder (red lofted hull, canopy bubble, side intakes, wing strakes,
twin tail fins, spine insignia, glowing tail pipe), modeled from the
gameplay captures in `../screenshots/`. Not a rip of game geometry —
a clean-room promo model.

## Files

- `render_craft.py` — the whole pipeline: procedural mesh + flat-shaded
  numpy software rasterizer (only dep: numpy). PPM to stdout/file.
- `hero-craft.png` — 800x600 3/4 publicity still.
- `craft-turntable.mp4` — 4 s seamless 360° loop, 800x600, 30 fps.
- `craft-turntable.gif` — GIF version (480 px, 12 fps).

## Regenerating

```sh
# one still (hero 3/4 view)
python3 render_craft.py 0 120 hero.ppm --hero

# full 120-frame loop, then encode (frames are regenerable scratch)
mkdir -p /tmp/frames
seq 0 119 | xargs -P 8 -I{} sh -c \
  'python3 render_craft.py {} 120 /tmp/frames/f$(printf "%03d" {}).ppm'
ffmpeg -y -framerate 30 -i /tmp/frames/f%03d.ppm -c:v libx264 \
  -preset medium -crf 20 -pix_fmt yuv420p -movflags +faststart \
  craft-turntable.mp4
```

Frames take ~1 s each single-threaded; the `xargs -P 8` pass above
finishes in well under a minute. Convert frames with
`convert f000.ppm f000.png` (ImageMagick) if stills are needed.
Background stars are seeded (`default_rng(1337)`), so every run is
bit-identical.
