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
LIGHT_BG = ((246, 248, 247), (226, 232, 229))    # wallpaper, top -> bottom
DARK_BG = ((35, 38, 41), (20, 22, 24))


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
    """The logo with its lettering replaced by FULL_NAME: same typeface, cap
    height, letter spacing, colour and baseline, centred under the mark."""
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

    mask, (x0, y0, x1, y1) = tracked_text(FULL_NAME, font, tracking)
    mask = mask.crop((x0, y0, x1, y1))
    centre = (cols[0] + cols[-1]) / 2
    margin = cols[0]
    w = max(src.width, mask.width + 2 * margin)
    img = Image.new("RGB", (w, src.height), "white")
    shift = round(w / 2 - centre)
    img.paste(src.crop((0, 0, src.width, top - 1)), (shift, 0))
    img.paste(Image.new("RGB", mask.size, colour), (round(w / 2 - mask.width / 2), top), mask)
    return img


def gradient(size, top, bottom):
    w, h = size
    t = np.linspace(0, 1, h)[:, None]
    col = np.array(top, float) * (1 - t) + np.array(bottom, float) * t
    return Image.fromarray(np.repeat(col[:, None, :], w, axis=1).round().astype(np.uint8), "RGB")


def compose(size, logo, bg, height=0.34):
    """The logo centred (slightly above the middle) on a vertical gradient."""
    w, h = size
    img = gradient(size, *bg).convert("RGBA")
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

    named = trim(color_to_alpha(full_name_logo()), 12)
    named_dark = invert_lightness(named)
    wp = OUT / "wallpapers/jk_os/contents"
    for w, h in ((1920, 1080), (2560, 1440), (3840, 2160), (1080, 1920)):
        save(compose((w, h), named, LIGHT_BG), wp / f"images/{w}x{h}.png")
        save(compose((w, h), named_dark, DARK_BG), wp / f"images_dark/{w}x{h}.png")
    save(compose((400, 250), named, LIGHT_BG), wp / "screenshot.png")

    pv = OUT / "plasma/look-and-feel/org.jk_os.desktop/contents/previews"
    full = compose((1920, 1080), named, LIGHT_BG)
    save(full, pv / "fullscreenpreview.jpg", quality=90)
    prev = full.resize((600, 337), Image.LANCZOS).convert("RGBA")
    ImageDraw.Draw(prev).rectangle([0, 337 - 16, 600, 337], fill=(239, 240, 241, 255))
    prev.alpha_composite(render_mark(12), (6, 337 - 14))
    save(prev, pv / "preview.png")
    save(compose((300, 169), logo, LIGHT_BG, height=0.42), pv / "splash.png")


if __name__ == "__main__":
    main()
