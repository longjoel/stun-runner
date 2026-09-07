#!/usr/bin/env python3
"""Procedural 3D turntable renderer for the S.T.U.N. Runner player craft.

Original low-poly interpretation of the tunnel speeder (red hull, canopy,
twin fins, engine glow). Pure-numpy software rasterizer, no third-party
deps beyond numpy. Emits binary PPM to stdout (or a file); convert to PNG
with ImageMagick, e.g. ``convert frame.ppm frame.png``.

Usage:
    python3 render_craft.py <frame_index> <num_frames> [out.ppm] [--hero]
    --hero renders a fixed 3/4 publicity still instead of a turntable frame.
"""

import sys

import numpy as np

W, H = 800, 600
FOCAL = H * 1.45
EYE = np.array([0.0, 2.05, 7.4])
LOOK = np.array([0.0, 0.15, 0.0])
LIGHT = np.array([0.5, 0.8, 0.45])
LIGHT = LIGHT / np.linalg.norm(LIGHT)

RED = np.array([0.78, 0.13, 0.11])
DARKRED = np.array([0.48, 0.07, 0.07])
CANOPY = np.array([0.82, 0.86, 0.92])
DARK = np.array([0.16, 0.18, 0.23])
FIN = np.array([0.60, 0.64, 0.71])
YELLOW = np.array([1.0, 0.83, 0.30])


def rot_y(theta):
    c, s = np.cos(theta), np.sin(theta)
    return np.array([[c, 0.0, s], [0.0, 1.0, 0.0], [-s, 0.0, c]])


class Mesh:
    def __init__(self):
        self.verts = []   # list of (3,) float
        self.tris = []    # list of (3,) int
        self.colors = []  # list of (3,) float, one per tri

    def v(self, p):
        self.verts.append(tuple(float(x) for x in p))
        return len(self.verts) - 1

    def tri(self, a, b, c, color):
        self.tris.append((a, b, c))
        self.colors.append(np.array(color, dtype=float))

    def quad(self, a, b, c, d, color):
        self.tri(a, b, c, color)
        self.tri(a, c, d, color)

    def box(self, cx, cy, cz, sx, sy, sz, color):
        x0, x1 = cx - sx / 2, cx + sx / 2
        y0, y1 = cy - sy / 2, cy + sy / 2
        z0, z1 = cz - sz / 2, cz + sz / 2
        i = [self.v(p) for p in
             [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
              (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]]
        for a, b, c, d in [(0, 1, 2, 3), (4, 5, 6, 7), (0, 4, 5, 1),
                           (2, 6, 7, 3), (1, 5, 6, 2), (0, 3, 7, 4)]:
            self.quad(i[a], i[b], i[c], i[d], color)

    def loft(self, sections, k=8, color=RED):
        """sections: list of (x, half_width, half_height, y_center)."""
        rings = []
        for x, hw, hh, yc in sections:
            ring = []
            for j in range(k):
                a = 2 * np.pi * j / k + np.pi / k
                ring.append(self.v((x, yc + hh * np.sin(a), hw * np.cos(a))))
            rings.append(ring)
        for r0, r1 in zip(rings[:-1], rings[1:]):
            for j in range(k):
                self.quad(r0[j], r0[(j + 1) % k],
                          r1[(j + 1) % k], r1[j], color)
        # tail cap fan
        c0 = self.v((sections[0][0] - 0.02, sections[0][3], 0.0))
        for j in range(k):
            self.tri(c0, rings[0][(j + 1) % k], rings[0][j], DARKRED)
        # nose cap fan
        c1 = self.v((sections[-1][0] + 0.28, sections[-1][3], 0.0))
        for j in range(k):
            self.tri(c1, rings[-1][j], rings[-1][(j + 1) % k], color)


def build_craft():
    m = Mesh()
    # main hull loft: nose toward +X
    m.loft([(-2.00, 0.55, 0.34, 0.05),
            (-1.20, 0.95, 0.50, 0.05),
            (-0.20, 1.02, 0.55, 0.05),
            (0.80, 0.78, 0.44, 0.02),
            (1.60, 0.42, 0.28, -0.02),
            (2.05, 0.10, 0.10, -0.04)], color=RED)
    # canopy bubble
    m.loft([(-0.95, 0.34, 0.20, 0.52),
            (-0.40, 0.44, 0.34, 0.55),
            (0.30, 0.36, 0.28, 0.50),
            (0.75, 0.12, 0.10, 0.42)], k=6, color=CANOPY)
    # side intakes (dark)
    m.box(0.05, -0.05, 1.00, 1.50, 0.34, 0.30, DARK)
    m.box(0.05, -0.05, -1.00, 1.50, 0.34, 0.30, DARK)
    # wing strakes
    for s in (1.0, -1.0):
        p = [m.v(q) for q in [(-1.35, 0.02, 0.55 * s), (0.55, -0.02, 0.80 * s),
                              (0.15, -0.02, 1.55 * s), (-1.15, 0.02, 1.30 * s)]]
        m.quad(p[0], p[1], p[2], p[3], DARKRED)
    # twin tail fins (swept, gray)
    for s in (1.0, -1.0):
        p = [m.v(q) for q in [(-2.05, 0.25, 0.42 * s), (-1.15, 0.30, 0.48 * s),
                              (-1.45, 1.25, 0.48 * s), (-2.00, 1.20, 0.42 * s)]]
        m.quad(p[0], p[1], p[2], p[3], FIN)
    # yellow spine insignia
    m.box(-0.35, 0.62, 0.0, 0.55, 0.05, 0.22, YELLOW)
    return m


def cam_basis():
    fwd = LOOK - EYE
    fwd = fwd / np.linalg.norm(fwd)
    right = np.cross(fwd, np.array([0.0, 1.0, 0.0]))
    right = right / np.linalg.norm(right)
    up = np.cross(right, fwd)
    return right, up, fwd


def project(pts, right, up, fwd):
    rel = pts - EYE
    z = rel @ fwd
    x = rel @ right
    y = rel @ up
    depth = rel @ fwd
    sx = x * FOCAL / depth + W / 2
    sy = -y * FOCAL / depth + H / 2
    return np.stack([sx, sy], axis=1), depth


def rasterize(img, zbuf, pts2d, depth, color):
    (x0, y0), (x1, y1), (x2, y2) = pts2d
    area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)
    if abs(area) < 1e-9:
        return
    xmin = max(int(np.floor(min(x0, x1, x2))), 0)
    xmax = min(int(np.ceil(max(x0, x1, x2))), W - 1)
    ymin = max(int(np.floor(min(y0, y1, y2))), 0)
    ymax = min(int(np.ceil(max(y0, y1, y2))), H - 1)
    if xmax < xmin or ymax < ymin:
        return
    ys, xs = np.mgrid[ymin:ymax + 1, xmin:xmax + 1]
    px = xs.astype(float) + 0.5
    py = ys.astype(float) + 0.5
    w0 = ((x1 - px) * (y2 - py) - (x2 - px) * (y1 - py)) / area
    w1 = ((x2 - px) * (y0 - py) - (x0 - px) * (y2 - py)) / area
    w2 = 1.0 - w0 - w1
    inside = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
    if not inside.any():
        return
    d = w0 * depth[0] + w1 * depth[1] + w2 * depth[2]
    sub = zbuf[ymin:ymax + 1, xmin:xmax + 1]
    upd = inside & (d < sub)
    sub[upd] = d[upd]
    patch = img[ymin:ymax + 1, xmin:xmax + 1]
    patch[upd] = np.clip(color, 0, 1)[None, None, :]


def background(rng):
    top = np.array([0.07, 0.05, 0.13])
    bot = np.array([0.02, 0.01, 0.04])
    t = (np.arange(H) / H)[:, None, None]
    img = (top * (1 - t) + bot * t) * np.ones((1, W, 1))
    img = np.repeat(img, 1, axis=1).reshape(H, W, 3)
    for _ in range(170):
        x = rng.integers(0, W)
        y = rng.integers(0, H)
        b = rng.uniform(0.25, 1.0)
        r = int(rng.integers(1, 3))
        img[max(0, y - r):y + r + 1, max(0, x - r):x + r + 1] += b * 0.35
    return np.clip(img, 0, 1)


def disc(img, cx, cy, radius, color, strength=1.0):
    ymin = max(int(cy - radius), 0)
    ymax = min(int(cy + radius), H - 1)
    xmin = max(int(cx - radius), 0)
    xmax = min(int(cx + radius), W - 1)
    if xmax < xmin or ymax < ymin:
        return
    ys, xs = np.mgrid[ymin:ymax + 1, xmin:xmax + 1]
    d = np.sqrt((xs - cx) ** 2 + (ys - cy) ** 2) / radius
    fall = np.clip(1 - d, 0, 1) ** 2 * strength
    patch = img[ymin:ymax + 1, xmin:xmax + 1]
    patch += (fall[..., None] * color)
    np.clip(patch, 0, 1, out=patch)


def render_frame(theta_deg, rng):
    theta = np.radians(theta_deg)
    mesh = build_craft()
    V = np.array(mesh.verts) @ rot_y(theta).T
    V[:, 1] += 0.10 + 0.07 * np.sin(2 * theta)  # gentle hover bob
    T = np.array(mesh.tris)
    C = np.array(mesh.colors)
    right, up, fwd = cam_basis()
    P2, depth = project(V, right, up, fwd)

    img = background(rng)
    zbuf = np.full((H, W), np.inf)

    # soft shadow on the floor plane under the craft
    sc, _ = project(np.array([[0.0, -0.85, 0.0]]), right, up, fwd)
    yy, xx = np.mgrid[0:H, 0:W]
    sh = np.exp(-(((xx - sc[0, 0]) / 150) ** 2 + ((yy - sc[0, 1]) / 34) ** 2))
    img *= (1 - 0.45 * sh[..., None])

    P = P2[T]              # (F,3,2)
    D = depth[T]           # (F,3)
    valid = (D > 0.5).all(axis=1)
    v0 = V[T[:, 0]]
    v1 = V[T[:, 1]]
    n = np.cross(v1 - v0, V[T[:, 2]] - v0)
    nl = np.linalg.norm(n, axis=1, keepdims=True) + 1e-12
    n = n / nl
    tri_c = (v0 + v1 + V[T[:, 2]]) / 3.0
    flip = ((n * (tri_c - V.mean(axis=0))).sum(axis=1) < 0)
    n[flip] *= -1
    view = (EYE - tri_c)
    view = view / (np.linalg.norm(view, axis=1, keepdims=True) + 1e-12)
    facing = (n * view).sum(axis=1) > 0
    lambert = np.clip(n @ LIGHT, 0, 1)
    shade = 0.38 + 0.62 * lambert
    order = np.argsort(D[:, :].mean(axis=1))[::-1]
    for i in order:
        if not (valid[i] and facing[i]):
            continue
        rasterize(img, zbuf, P[i], D[i], C[i] * shade[i])

    # engine glow (additive sprite at the tail pipe)
    eng = (rot_y(theta) @ np.array([-2.02, 0.05, 0.0]))
    eng[1] += 0.10 + 0.07 * np.sin(2 * theta)
    pe, de = project(eng[None, :], right, up, fwd)
    if de[0] > 0.5:
        r = FOCAL * 0.16 / de[0]
        disc(img, pe[0, 0], pe[0, 1], r, np.array([1.0, 0.45, 0.10]), 0.9)
        disc(img, pe[0, 0], pe[0, 1], r * 0.45,
             np.array([1.0, 0.90, 0.65]), 1.0)
    return np.clip(img, 0, 1)


def main(argv):
    hero = "--hero" in argv
    argv = [a for a in argv if a != "--hero"]
    idx = int(argv[0]) if len(argv) > 0 else 0
    n = int(argv[1]) if len(argv) > 1 else 120
    out = argv[2] if len(argv) > 2 else None
    rng = np.random.default_rng(1337)
    theta = -45.0 if hero else 360.0 * idx / n
    img = render_frame(theta, rng)
    ppm = (b"P6\n%d %d\n255\n" % (W, H)) + (img * 255).astype(np.uint8).tobytes()
    if out:
        with open(out, "wb") as f:
            f.write(ppm)
    else:
        sys.stdout.buffer.write(ppm)


if __name__ == "__main__":
    main(sys.argv[1:])
