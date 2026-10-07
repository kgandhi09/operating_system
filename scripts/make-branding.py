#!/usr/bin/env python3
"""Generate jk_os's desktop branding from the J.K. Robotics logo.

    scripts/make-branding.py

Reads "J.K. Logo.png" at the top of the repo and writes, under rootfs/:

  usr/share/jk_os/logo.png, logo-dark.png   the logo on transparency, for
                                            light and dark backgrounds
  usr/share/icons/hicolor/<n>x<n>/apps/jk-os.png
                                            the JK mark as an icon (app
                                            launcher, os-release LOGO)
  usr/share/wallpapers/jk_os/               the default wallpaper (light and
                                            dark variants): the mark over the
                                            company's full name, set in the
                                            logo's typeface (Poppins Light,
                                            scripts/branding, SIL OFL)
  usr/share/plasma/look-and-feel/org.jk_os.desktop/contents/previews/
                                            the Global Theme's previews

The outputs are committed, so building the OS needs neither Pillow nor
NumPy; rerun this after changing the logo.
"""
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "J.K. Logo.png"
FONT = ROOT / "scripts/branding/Poppins-Light.ttf"
LOGO_TEXT = "JK ROBOTICS"              # the name as the logo spells it
FULL_NAME = "J.K. ROBOTICS PVT. LTD."  # the wallpaper's
OUT = ROOT / "rootfs/usr/share"

GREEN = np.array([62, 178, 74], float)
DARK = np.array([68, 69, 69], float)
LIGHT_BG = ((246, 248, 247), (226, 232, 229))    # plain backgrounds, top -> bottom
DARK_BG = ((35, 38, 41), (20, 22, 24))
# The wallpaper: soft glows of colour, kept away from the logo, on a gradient
# (what shows through the desktop's glass panels). Each glow: centre (x, y,
# as fractions of the width and height), radius (fraction of the width),
# colour, strength.
AURORA_LIGHT = dict(base=((236, 243, 240), (221, 231, 238)), glows=[
    (0.10, 0.18, 0.42, (150, 226, 176), 0.85),     # mint, top left
    (0.92, 0.10, 0.40, (150, 200, 250), 0.80),     # sky, top right
    (0.86, 0.92, 0.46, (205, 190, 250), 0.70),     # lilac, bottom right
    (0.14, 0.95, 0.40, (252, 214, 180), 0.60),     # peach, bottom left
    (0.55, 1.10, 0.30, (160, 230, 220), 0.45),     # aqua, bottom
])
AURORA_DARK = dict(base=((12, 16, 24), (6, 8, 13)), glows=[
    (0.08, 0.90, 0.50, (34, 150, 70), 0.75),       # the logo's green, bottom left
    (0.95, 0.12, 0.45, (18, 104, 140), 0.70),      # teal, top right
    (0.80, 0.98, 0.42, (70, 48, 150), 0.65),       # indigo, bottom right
    (0.10, 0.05, 0.38, (24, 56, 120), 0.60),       # deep blue, top left
    (0.50, -0.15, 0.30, (40, 120, 110), 0.35),     # a green-blue glow above
])


def color_to_alpha(img):
    """The logo is drawn on opaque white: turn the white into transparency."""
    c = np.asarray(img.convert("RGB"), float) / 255
    a = (1 - c).max(axis=2)
    safe = np.where(a > 0, a, 1)[..., None]
    rgb = np.clip(1 - (1 - c) / safe, 0, 1)
    return Image.fromarray((np.dstack([rgb, a]) * 255).round().astype(np.uint8), "RGBA")


def invert_lightness(img):
    """Same hue and saturation, lightness mirrored: the dark strokes and text
    turn light, the green stays green. For dark backgrounds."""
    c = np.asarray(img, float) / 255
    rgb, a = c[..., :3], c[..., 3:]
    rgb = rgb + (1 - rgb.max(axis=2, keepdims=True) - rgb.min(axis=2, keepdims=True))
    return Image.fromarray((np.dstack([np.clip(rgb, 0, 1), a]) * 255).round().astype(np.uint8), "RGBA")


def trim(img, margin):
    x0, y0, x1, y1 = img.getchannel("A").point(lambda v: 255 if v > 8 else 0).getbbox()
    return img.crop((x0 - margin, y0 - margin, x1 + margin, y1 + margin))


def tracked_text(text, font, tracking):
    """The text's coverage (L), letter by letter with extra spacing, and the
    bounding box of its ink."""
    width = round(sum(font.getlength(ch) for ch in text) + tracking * len(text)) + 4
    size = font.size * 2
    mask = Image.new("L", (width, size))
    d = ImageDraw.Draw(mask)
    x = 0.0
    for ch in text:
        d.text((x, size // 4), ch, font=font, fill=255)
        x += font.getlength(ch) + tracking
    return mask, mask.getbbox()


def full_name_logo():
    """Render the mark and full name at twice the source resolution.

    The supplied PNG provides the lettering's proportions and colour. The
    wallpaper itself uses the mark's paths and the Poppins font directly, so
    its edges do not inherit the source PNG's 700-pixel resolution.
    """
    scale = 2
    src = Image.open(SRC).convert("RGB")
    ink = np.asarray(src.convert("L")) < 200
    rows = np.flatnonzero(ink.any(axis=1))
    top = rows[np.flatnonzero(np.diff(rows) > 1)[-1] + 1]      # the lettering's first row
    bottom = rows[-1]
    cols = np.flatnonzero(ink[top:bottom + 1].any(axis=0))
    cap, width = bottom - top + 1, cols[-1] - cols[0] + 1
    block = np.asarray(src)[top:bottom + 1]
    colour = tuple(int(v) for v in block.reshape(-1, 3)[block.sum(axis=2).argmin()])

    # Match the logo's lettering: the size from its height, the spacing from its width.
    font = ImageFont.truetype(str(FONT), 100)
    _, (x0, y0, x1, y1) = tracked_text(LOGO_TEXT, font, 0)
    font = ImageFont.truetype(str(FONT), round(100 * cap / (y1 - y0)))
    _, (x0, y0, x1, y1) = tracked_text(LOGO_TEXT, font, 0)
    tracking = (width - (x1 - x0)) / (len(LOGO_TEXT) - 1)

    font = ImageFont.truetype(str(FONT), font.size * scale)
    mask, (x0, y0, x1, y1) = tracked_text(FULL_NAME, font, tracking * scale)
    mask = mask.crop((x0, y0, x1, y1))
    centre = (cols[0] + cols[-1]) / 2 * scale
    margin = cols[0] * scale
    w = max(src.width * scale, mask.width + 2 * margin)
    img = Image.new("RGBA", (w, src.height * scale))
    mark = Image.new("RGBA", (src.width * scale, top * scale))
    draw = ImageDraw.Draw(mark)

    def mark_colour(t):
        return tuple(round(v) for v in GREEN * (1 - t) + DARK * t) + (255,)

    stroke_width = round(STROKE * scale)
    for points, tones in STROKES:
        for (p, tp), (q, tq) in zip(zip(points, tones), zip(points[1:], tones[1:])):
            steps = max(abs(q[0] - p[0]), abs(q[1] - p[1])) // 3 + 1
            for i in range(steps):
                t0, t1 = i / steps, (i + 1) / steps
                a = ((p[0] + (q[0] - p[0]) * t0) * scale,
                     (p[1] + (q[1] - p[1]) * t0) * scale)
                b = ((p[0] + (q[0] - p[0]) * t1) * scale,
                     (p[1] + (q[1] - p[1]) * t1) * scale)
                draw.line((a, b), fill=mark_colour(tp + (tq - tp) * (t0 + t1) / 2), width=stroke_width)
            for point, tone in ((p, tp), (q, tq)):
                x, y = point[0] * scale, point[1] * scale
                r = stroke_width / 2
                draw.ellipse((x - r, y - r, x + r, y + r), fill=mark_colour(tone))
    for (x, y), tone in RINGS:
        radius, line = RING_R * scale, round(RING_W * scale)
        x, y = x * scale, y * scale
        draw.ellipse((x - radius, y - radius, x + radius, y + radius),
                     fill=(0, 0, 0, 0), outline=mark_colour(tone), width=line)

    img.alpha_composite(mark, (round(w / 2 - centre), 0))
    img.paste(Image.new("RGBA", mask.size, colour + (255,)),
              (round(w / 2 - mask.width / 2), top * scale), mask)
    return img


def gradient(size, top, bottom):
    w, h = size
    t = np.linspace(0, 1, h)[:, None]
    col = np.array(top, float) * (1 - t) + np.array(bottom, float) * t
    return Image.fromarray(np.repeat(col[:, None, :], w, axis=1).round().astype(np.uint8), "RGB")


def aurora(size, spec, seed=7):
    """Glows of colour over a gradient, with a little noise against banding."""
    w, h = size
    base = np.asarray(gradient(size, *spec["base"]), float)
    yy, xx = np.mgrid[0:h, 0:w]
    for cx, cy, r, colour, a in spec["glows"]:
        d2 = ((xx - cx * w) ** 2 + (yy - cy * h) ** 2) / (r * w) ** 2
        k = (a * np.exp(-2.2 * d2))[..., None]
        base = base * (1 - k) + np.array(colour, float) * k
    base += np.random.default_rng(seed).uniform(-1.2, 1.2, base.shape)
    return Image.fromarray(base.round().clip(0, 255).astype(np.uint8), "RGB")


def compose(size, logo, bg, height=0.34):
    """The logo centred (slightly above the middle) on a vertical gradient,
    or on glows (bg: an AURORA_* spec)."""
    w, h = size
    img = (aurora(size, bg) if isinstance(bg, dict) else gradient(size, *bg)).convert("RGBA")
    lh = round(min(h * height, w * height * 0.9))
    lw = round(logo.width * lh / logo.height)
    lg = logo.resize((lw, lh), Image.LANCZOS)
    img.alpha_composite(lg, ((w - lw) // 2, (h - lh) // 2 - h // 30))
    return img.convert("RGB")


# ---------------------------------------------------------------- the mark
# The JK mark, redrawn as vectors (coordinates in the source image) so small
# icons can have thicker strokes than a plain downscale would give them.
# Each stroke: a polyline and the colour at each point (0 = green, 1 = dark).
CROSS = (378, 320)
STROKES = [
    ([(293, 121), (293, 363), (117, 540)], [1, 1, 0]),       # J: dark, to green
    ([(335, 36), (335, 277), CROSS], [0, 0, 0.25]),           # K stem, top half
    ([(598, 103), CROSS, (335, 363), (335, 521)], [1, 0.25, 0, 0]),
    ([CROSS, (598, 540)], [0.25, 1]),                          # K leg
]
RINGS = [((293, 112), 1), ((335, 27), 0), ((335, 530), 0)]    # centre, colour
MARK_BOX = (80, 0, 630, 550)       # square-ish frame around the mark
STROKE, RING_R, RING_W = 10.5, 9, 3.2


def render_mark(size):
    ss = 4                                        # supersampling
    px = size * ss
    x0, y0, x1, y1 = MARK_BOX
    scale = px * 0.94 / max(x1 - x0, y1 - y0)
    ox = (px - (x1 - x0) * scale) / 2 - x0 * scale
    oy = (px - (y1 - y0) * scale) / 2 - y0 * scale
    # At icon sizes the source strokes would be sub-pixel: keep them >= ~1.4px.
    boost = max(1.0, 1.4 * ss / (STROKE * scale))
    sw, rr, rw = STROKE * scale * boost, RING_R * scale * boost, RING_W * scale * boost
    rw = max(rw, 1.0 * ss)
    rr = max(rr, sw * 0.75)

    def pt(p):
        return (p[0] * scale + ox, p[1] * scale + oy)

    # Colour field: each stroke paints its own gradient through its mask.
    img = np.zeros((px, px, 4))
    for pts, ts in STROKES:
        for (p, tp), (q, tq) in zip(zip(pts, ts), zip(pts[1:], ts[1:])):
            p, q = pt(p), pt(q)
            mask = Image.new("L", (px, px))
            d = ImageDraw.Draw(mask)
            d.line([p, q], fill=255, width=round(sw))
            for c in (p, q):      # round joins inside a polyline
                r = sw / 2
                d.ellipse([c[0] - r, c[1] - r, c[0] + r, c[1] + r], fill=255)
            m = np.asarray(mask, float)[..., None] / 255
            yy, xx = np.mgrid[0:px, 0:px]
            v = np.array(q) - np.array(p)
            t = np.clip(((xx - p[0]) * v[0] + (yy - p[1]) * v[1]) / (v @ v), 0, 1)[..., None]
            k = tp + (tq - tp) * t
            col = GREEN * (1 - k) + DARK * k
            layer = np.dstack([col, np.full((px, px), 255.0)]) * m
            img = layer + img * (1 - m)
    # The end rings are too small to read below 64px: small icons end the
    # strokes plainly.
    for c, k in (RINGS if size >= 64 else []):
        mask = Image.new("L", (px, px))
        cx, cy = pt(c)
        ImageDraw.Draw(mask).ellipse([cx - rr, cy - rr, cx + rr, cy + rr], outline=255, width=round(rw))
        m = np.asarray(mask, float)[..., None] / 255
        col = GREEN * (1 - k) + DARK * k
        # Rings sit on the stroke ends: clear the inside, then draw the ring.
        hole = Image.new("L", (px, px))
        ir = rr - rw
        ImageDraw.Draw(hole).ellipse([cx - ir, cy - ir, cx + ir, cy + ir], fill=255)
        h = np.asarray(hole, float)[..., None] / 255
        img = img * (1 - h)
        img = np.dstack([np.broadcast_to(col, (px, px, 3)), np.full((px, px), 255.0)]) * m + img * (1 - m)
    # Un-premultiply, then downsample.
    a = img[..., 3:]
    rgb = np.where(a > 0, img[..., :3] / np.where(a > 0, a, 1) * 255, 0)
    out = Image.fromarray(np.dstack([rgb, a]).round().clip(0, 255).astype(np.uint8), "RGBA")
    return out.resize((size, size), Image.LANCZOS)


def save(img, path, **kw):
    path.parent.mkdir(parents=True, exist_ok=True)
    img.save(path, optimize=True, **kw)
    print(path.relative_to(ROOT))


def main():
    logo = trim(color_to_alpha(Image.open(SRC)), 12)
    logo_dark = invert_lightness(logo)
    save(logo, OUT / "jk_os/logo.png")
    save(logo_dark, OUT / "jk_os/logo-dark.png")

    for n in (16, 22, 24, 32, 48, 64, 96, 128, 256, 512):
        save(render_mark(n), OUT / f"icons/hicolor/{n}x{n}/apps/jk-os.png")

    named = trim(full_name_logo(), 24)
    named_dark = invert_lightness(named)
    # JPEG: the glows' fine noise (against banding) doesn't compress as PNG.
    wp = OUT / "wallpapers/jk_os/contents"
    for names, size in (
            (((1920, 1080), (2560, 1440), (3840, 2160)), (3840, 2160)),
            (((1080, 1920), (2160, 3840)), (2160, 3840))):
        light = compose(size, named, AURORA_LIGHT)
        dark = compose(size, named_dark, AURORA_DARK)
        for label in names:
            filename = f"{label[0]}x{label[1]}.jpg"
            save(light, wp / "images" / filename, quality=98, subsampling=0)
            save(dark, wp / "images_dark" / filename, quality=98, subsampling=0)
    save(compose((400, 250), named, AURORA_LIGHT), wp / "screenshot.png")

    # The Global Themes' previews (light and dark): the wallpaper, a menu bar
    # along the top and a dock at the bottom, in glass.
    for package, img_logo, spec, glass, text in (
            ("org.jk_os.desktop", named, AURORA_LIGHT, (245, 245, 248, 150), (29, 29, 31, 255)),
            ("org.jk_os.desktop.dark", named_dark, AURORA_DARK, (40, 40, 44, 150), (242, 242, 247, 255))):
        pv = OUT / f"plasma/look-and-feel/{package}/contents/previews"
        full = compose((1920, 1080), img_logo, spec)
        save(full, pv / "fullscreenpreview.jpg", quality=90)
        prev = full.resize((600, 337), Image.LANCZOS).convert("RGBA")
        layer = Image.new("RGBA", prev.size)
        d = ImageDraw.Draw(layer)
        d.rectangle([0, 0, 600, 11], fill=glass)
        d.rounded_rectangle([222, 306, 378, 330], radius=7, fill=glass)
        for i, c in enumerate(((66, 133, 244), (255, 149, 0), (40, 40, 44), (142, 142, 147), (62, 178, 74))):
            d.rounded_rectangle([230 + i * 30, 310, 246 + i * 30, 326], radius=4, fill=c + (255,))
        d.text((560, 0), "9:41", fill=text)
        prev.alpha_composite(layer)
        prev.alpha_composite(render_mark(10), (4, 1))
        save(prev, pv / "preview.png")
        save(compose((300, 169), img_logo, spec, height=0.42), pv / "splash.png")


if __name__ == "__main__":
    main()
