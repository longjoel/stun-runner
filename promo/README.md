# Promo kit — S.T.U.N. Runner reverse-engineering project

Original project art plus real captures from the pinned MAME oracle.
All vector art here is original (tunnel-and-craft motif); gameplay pixels
are captures of S.T.U.N. Runner (rev 6) running under pinned MAME 0.289.

## The goods

| File | What it is | Size |
|------|------------|------|
| `sizzle-reel.mp4` | 21 s, 1280x720, 30 fps promo cut (silent) | ~1.6 MB |
| `sizzle-reel-preview.gif` | 6 s GIF preview of the reel (480 px) | — |
| `logo.svg` / `logo.png` / `logo-small.png` | Project logo: vector master, 1200x630, 400x210 | — |
| `social-card.png` | 1200x630 share card | — |
| `poster.png` | 1080x1350 poster | — |
| `wallpaper.png` | 1920x1080 desktop wallpaper | — |
| `sticker.svg` / `sticker.png` | Die-cut style sticker, 600x600 | — |
| `banner.txt` | ASCII banner for terminals/README headers | — |
| `screenshots/shot-tunnel-00.png` | Full-frame tunnel gameplay capture, 1280x720 | — |
| `screenshots/shot-craft-closeup.png` | Craft close-up crop, 1280x720 | — |
| `screenshots/shot-lab-vram.png` | "Under the hood" GSP VRAM decode artifact | — |
| `FACTS.md` | One-page hype sheet with real project facts | — |
| `craft3d/render_craft.py` | Procedural 3D craft model + numpy renderer (regen script) | — |
| `craft3d/hero-craft.png` | 800x600 3/4 still of the 3D craft | — |
| `craft3d/craft-turntable.mp4` | 4 s seamless 360° craft rotation, 800x600/30fps | ~0.2 MB |
| `craft3d/craft-turntable.gif` | GIF version of the turntable (480 px) | — |

## Reel rundown (21 s, silent)

1. Title card — logo + "An evidence-driven resurrection" (0–3 s)
2. Gameplay zoom — the oracle under pinned MAME 0.289 (3–8 s)
3. Craft close-up pan — "M5: first visible output" (8–13 s)
4. Lab card — GSP VRAM decode experiments (13–17 s)
5. End card — 68010 · GSP · ADSP-2100 · 6502 (17–21 s)

Play it: `ffplay promo/sizzle-reel.mp4` (or any video player).

## Regenerating

Requires ImageMagick (`convert`) with the RSVG delegate, `ffmpeg`
(with libfreetype), and Liberation Sans. Rasters are derived from
`logo.svg` / `sticker.svg` and from local captures
(`snap/stunrun/*.png`, analysis VRAM artifacts), so re-running the
commands in shell history rebuilds everything. Fresh gameplay captures
come from the harness: `tools/mame-replay` with
`experiments/stunrun/boot_to_title.json` or `coin_start.json`.

## Notes

- Captures are of the original game running in MAME (the project's
  behavioral oracle), used here to promote the reconstruction effort.
- No ROM contents are committed anywhere in this repo; screenshots are
  the only game pixels present, same as the existing evidence policy.
