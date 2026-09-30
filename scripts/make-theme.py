#!/usr/bin/env python3
"""Generate jk_os's desktop theme, in a light and a dark variant.

    scripts/make-theme.py

Writes, under rootfs/usr/share:

  color-schemes/JkOsLight.colors, JkOsDark.colors
        the colours: neutral greys as on macOS, the J.K. Robotics green as the
        accent, Apple's system colours for errors, warnings and success
  aurorae/themes/jk_os-light, jk_os-dark
        the window title bars (KWin's Aurorae engine, with jk_os's patch for
        rounded corners): red, yellow and green buttons on the left that show
        their symbols when hovered, the title in the middle, a translucent bar
        KWin blurs, a soft shadow
  plasma/desktoptheme/jk_os
        the Plasma style: Breeze's, with panels, pop-ups and tooltips more
        translucent (KWin blurs and saturates what is behind them) and a dock
        indicator as on macOS (a small pill under running applications); it
        follows the colour scheme, so it serves both variants

The outputs are committed, so building the OS needs neither Pillow nor
NumPy; rerun this after changing it. The Global Themes that put these
together (org.jk_os.desktop, org.jk_os.desktop.dark) are plain files under
rootfs/usr/share/plasma/look-and-feel.
"""
import base64
import io
import json
import re
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "rootfs/usr/share"
BREEZE_THEME = ROOT / "userspace/desktop/libplasma/src/desktoptheme/breeze"

# ---------------------------------------------------------------- palettes
GREEN = (62, 178, 74)           # the logo's green
GREEN_DEEP = (40, 150, 60)      # the same, dark enough for white text

LIGHT = dict(
    name="jk_os Light", id="JkOsLight",
    window=(242, 242, 245), window_alt=(232, 232, 236),
    view=(255, 255, 255), view_alt=(246, 246, 248),
    button=(255, 255, 255), button_alt=(214, 238, 218),
    header=(236, 236, 240), header_alt=(226, 226, 231),
    header_inactive=(244, 244, 246),
    tooltip=(250, 250, 252),
    text=(29, 29, 31), text_inactive=(128, 128, 134),
    accent=GREEN_DEEP, accent_text=(255, 255, 255), accent_alt=(186, 226, 192),
    link=(0, 113, 227), visited=(137, 68, 171),
    negative=(215, 58, 73), neutral=(214, 125, 0), positive=(36, 150, 64),
    complementary=(28, 28, 30), complementary_text=(242, 242, 247),
)
DARK = dict(
    name="jk_os Dark", id="JkOsDark",
    window=(30, 30, 32), window_alt=(38, 38, 41),
    view=(22, 22, 24), view_alt=(28, 28, 31),
    button=(56, 56, 60), button_alt=(34, 74, 42),
    header=(40, 40, 44), header_alt=(34, 34, 37),
    header_inactive=(32, 32, 35),
    tooltip=(44, 44, 48),
    text=(242, 242, 247), text_inactive=(150, 150, 157),
    accent=GREEN, accent_text=(255, 255, 255), accent_alt=(30, 84, 40),
    link=(64, 156, 255), visited=(191, 90, 242),
    negative=(255, 69, 58), neutral=(255, 159, 10), positive=(50, 215, 75),
    complementary=(18, 18, 20), complementary_text=(242, 242, 247),
)


def rgb(c):
    return ",".join(str(v) for v in c)


def hexc(c, a=None):
    s = "#%02x%02x%02x" % tuple(c)
    return s if a is None else s + "%02x" % round(a * 255)


def mix(a, b, t):
    return tuple(round(x * (1 - t) + y * t) for x, y in zip(a, b))


def save_text(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)
    print(path.relative_to(ROOT))


# ---------------------------------------------------------------- colour schemes
def color_group(p, bg, bg_alt, fg=None, fg_inactive=None):
    fg = fg or p["text"]
    fg_inactive = fg_inactive or p["text_inactive"]
    return {
        "BackgroundAlternate": bg_alt, "BackgroundNormal": bg,
        "DecorationFocus": p["accent"], "DecorationHover": p["accent"],
        "ForegroundActive": p["accent"], "ForegroundInactive": fg_inactive,
        "ForegroundLink": p["link"], "ForegroundNegative": p["negative"],
        "ForegroundNeutral": p["neutral"], "ForegroundNormal": fg,
        "ForegroundPositive": p["positive"], "ForegroundVisited": p["visited"],
    }


def color_scheme(p):
    groups = {
        "Colors:Button": color_group(p, p["button"], p["button_alt"]),
        "Colors:Complementary": color_group(p, p["complementary"], mix(p["complementary"], (0, 0, 0), 0.3),
                                            p["complementary_text"], (160, 160, 166)),
        "Colors:Header": color_group(p, p["header"], p["header_alt"]),
        "Colors:Header][Inactive": color_group(p, p["header_inactive"], p["header"]),
        "Colors:Selection": {**color_group(p, p["accent"], p["accent_alt"], p["accent_text"], p["text_inactive"]),
                             "ForegroundActive": p["accent_text"], "ForegroundLink": (255, 235, 160)},
        "Colors:Tooltip": color_group(p, p["tooltip"], p["window"]),
        "Colors:View": color_group(p, p["view"], p["view_alt"]),
        "Colors:Window": color_group(p, p["window"], p["window_alt"]),
    }
    dark = p is DARK
    out = ["# jk_os's colour scheme, generated by scripts/make-theme.py.", ""]
    out += ["[ColorEffects:Disabled]", "Color=56,56,56", "ColorAmount=0", "ColorEffect=0",
            "ContrastAmount=0.65", "ContrastEffect=1", "IntensityAmount=0.1", "IntensityEffect=2", ""]
    out += ["[ColorEffects:Inactive]", "ChangeSelectionColor=true", "Color=112,111,110",
            "ColorAmount=0.025", "ColorEffect=2", "ContrastAmount=0.1", "ContrastEffect=2",
            "Enable=false", "IntensityAmount=0", "IntensityEffect=0", ""]
    for name, keys in groups.items():
        out.append(f"[{name}]")
        out += [f"{k}={rgb(v)}" for k, v in keys.items()]
        out.append("")
    out += ["[General]", f"ColorScheme={p['id']}", f"Name={p['name']}", "shadeSortColumn=true", "",
            "[KDE]", "contrast=4", "",
            "[WM]",
            f"activeBackground={rgb(p['header'])}", f"activeBlend={rgb(p['text'] if dark else p['header'])}",
            f"activeForeground={rgb(p['text'])}",
            f"inactiveBackground={rgb(p['header_inactive'])}",
            f"inactiveBlend={rgb(p['text_inactive'] if dark else p['header_inactive'])}",
            f"inactiveForeground={rgb(p['text_inactive'])}", ""]
    return "\n".join(out)


# ---------------------------------------------------------------- title bars
SS = 2                          # the raster parts are drawn at twice the size
PAD_L, PAD_R, PAD_T, PAD_B = 28, 28, 18, 40     # room for the shadow
RADIUS = 10                     # window corners (KWin rounds the content too)
TITLE = 34                      # title bar height: edges 6 + 22 + 6
MID = 8                         # the stretched middle of each edge

BUTTON = 14                     # button box; the light is 12 across
LIGHTS = {                      # fill, rim, symbol
    "close": ((255, 95, 87), (224, 68, 62), (77, 0, 0)),
    "minimize": ((254, 188, 46), (222, 161, 35), (153, 87, 0)),
    "maximize": ((40, 200, 64), (26, 171, 41), (0, 100, 0)),
}


def rounded_rect_alpha(w, h, box, r, ss=SS):
    """Anti-aliased coverage (0..1) of a rounded rectangle box=(x0,y0,x1,y1)."""
    x0, y0, x1, y1 = (v * ss for v in box)
    r *= ss
    yy, xx = np.mgrid[0:h * ss, 0:w * ss] + 0.5
    # signed distance to the rounded rectangle's edge (negative inside)
    qx = np.abs(xx - (x0 + x1) / 2) - ((x1 - x0) / 2 - r)
    qy = np.abs(yy - (y0 + y1) / 2) - ((y1 - y0) / 2 - r)
    d = np.hypot(np.maximum(qx, 0), np.maximum(qy, 0)) + np.minimum(np.maximum(qx, qy), 0) - r
    return np.clip(0.5 - d, 0, 1)


def frame_image(p, active):
    """The whole decoration frame (shadow, rounded window, title bar) for a
    small reference window, RGBA at SS times the size."""
    w = PAD_L + RADIUS + MID + RADIUS + PAD_R
    h = PAD_T + TITLE + MID + RADIUS + PAD_B
    win = (PAD_L, PAD_T, w - PAD_R, h - PAD_B)
    W, H = w * SS, h * SS
    shape = rounded_rect_alpha(w, h, win, RADIUS)

    # Shadow: a wide soft one, offset down, and a tight one, outside the window only.
    dark = p is DARK
    strength = (0.55 if dark else 0.30) if active else (0.35 if dark else 0.16)
    shadow = np.zeros((H, W))
    for sigma, dy, a in ((14, 8, strength), (2, 1, strength * 0.6)):
        m = Image.fromarray((shape * 255).astype(np.uint8), "L")
        m = m.transform(m.size, Image.AFFINE, (1, 0, 0, 0, 1, -dy * SS))
        m = m.filter(ImageFilter.GaussianBlur(sigma * SS))
        shadow = np.maximum(shadow, np.asarray(m, float) / 255 * a)
    shadow *= 1 - shape

    # The window: title bar (translucent: KWin blurs behind it) over the body.
    # Close to opaque, so the bar reads as one with the application's toolbar
    # below it (the same Header colour), with the blur just showing through.
    title_alpha = 0.88 if active else 0.80
    head = np.array(p["header"] if active else p["header_inactive"], float)
    yy = np.arange(H)[:, None] / SS
    in_title = (yy >= PAD_T) & (yy < PAD_T + TITLE)
    t = np.clip((yy - PAD_T) / TITLE, 0, 1)
    # A faint sheen: slightly lighter at the top of the bar.
    sheen = np.where(in_title, (1 - t) * (0.04 if dark else 0.12), 0)
    colour = head[None, None, :] * (1 - sheen[..., None]) + 255 * sheen[..., None]
    body = np.array(p["window"], float)
    colour = np.where(in_title[..., None], colour, body[None, None, :])
    alpha = np.where(in_title, title_alpha, 1.0) * shape
    # A light rim along the top edge (the glass catching the light).
    rim = (yy >= PAD_T) & (yy < PAD_T + 1)
    lift = 0.2 if dark else 0.4
    colour = np.where(rim[..., None], colour * (1 - lift) + 255 * lift, colour)

    # The window over its shadow ("over" compositing onto black).
    out_a = alpha + shadow * (1 - alpha)
    safe = np.where(out_a > 0, out_a, 1)
    rgb_out = colour * alpha[..., None] / safe[..., None]
    img = Image.fromarray(np.dstack([rgb_out, out_a[..., None] * 255]).round().clip(0, 255).astype(np.uint8), "RGBA")
    return img, (w, h), shape


def png_uri(img):
    buf = io.BytesIO()
    img.save(buf, "PNG", optimize=True)
    return "data:image/png;base64," + base64.b64encode(buf.getvalue()).decode()


def nine_tiles(img, size, left, top, right, bottom):
    """Slice a frame image into FrameSvg's nine parts: (name, x, y, w, h, image)."""
    w, h = size
    xs = [0, left, w - right, w]
    ys = [0, top, h - bottom, h]
    names = [["topleft", "top", "topright"], ["left", "center", "right"],
             ["bottomleft", "bottom", "bottomright"]]
    for r in range(3):
        for c in range(3):
            x0, x1, y0, y1 = xs[c], xs[c + 1], ys[r], ys[r + 1]
            yield names[r][c], x0, y0, x1 - x0, y1 - y0, img.crop((x0 * SS, y0 * SS, x1 * SS, y1 * SS))


def svg_doc(width, height, body):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" '
            f'width="{width}" height="{height}" viewBox="0 0 {width} {height}">\n'
            f'<!-- Generated by jk_os\'s scripts/make-theme.py. -->\n{body}</svg>\n')


def decoration_svg(p):
    parts, y = [], 0
    left, top, right, bottom = PAD_L + RADIUS, PAD_T + TITLE, RADIUS + PAD_R, RADIUS + PAD_B
    for prefix, active in (("decoration", True), ("decoration-inactive", False)):
        img, size, shape = frame_image(p, active)
        for name, x, ty, w, h, tile in nine_tiles(img, size, left, top, right, bottom):
            parts.append(f'<image id="{prefix}-{name}" x="{x}" y="{y + ty}" width="{w}" height="{h}" '
                         f'preserveAspectRatio="none" xlink:href="{png_uri(tile)}"/>')
        y += size[1] + 4
    # Blur behind the title bar only (the rest is under the application).
    w, h = size
    mask = np.zeros((h * SS, w * SS))
    title_rows = slice(PAD_T * SS, (PAD_T + TITLE) * SS)
    mask[title_rows] = shape[title_rows]
    mimg = Image.fromarray((np.dstack([np.zeros(mask.shape + (3,)), mask[..., None]]) * 255).astype(np.uint8), "RGBA")
    for name, x, ty, tw, th, tile in nine_tiles(mimg, size, left, top, right, bottom):
        parts.append(f'<image id="mask-{name}" x="{x}" y="{y + ty}" width="{tw}" height="{th}" '
                     f'preserveAspectRatio="none" xlink:href="{png_uri(tile)}"/>')
    y += h + 4
    # Maximized: no corners or shadow, just the bar across the screen.
    for prefix, active in (("decoration-maximized", True), ("decoration-maximized-inactive", False)):
        c = p["header"] if active else p["header_inactive"]
        parts.append(f'<rect id="{prefix}-center" x="0" y="{y}" width="16" height="16" '
                     f'style="fill:{hexc(c)};fill-opacity:{0.86 if active else 0.8}"/>')
        y += 20
    parts.append(f'<rect id="hint-stretch-borders" x="0" y="{y}" width="2" height="2" style="fill:none"/>')
    return svg_doc(w, y + 4, "\n".join(parts) + "\n")


def button_svg(kind, p):
    """One title bar button: states active, hover, pressed, inactive, deactivated."""
    dark = p is DARK
    light = "maximize" if kind == "restore" else kind
    fill, rim, sym = LIGHTS[light]
    grey = (78, 78, 82) if dark else (213, 213, 216)
    grey_rim = (96, 96, 100) if dark else (190, 190, 194)
    c = BUTTON / 2
    r = 6

    def symbol(cy):
        """The symbol's path, centred at (c, cy)."""
        if kind == "close":
            return f"M {c-2.6} {cy-2.6} L {c+2.6} {cy+2.6} M {c+2.6} {cy-2.6} L {c-2.6} {cy+2.6}"
        if kind == "minimize":
            return f"M {c-3.2} {cy} L {c+3.2} {cy}"
        if kind == "maximize":     # two triangles pointing out
            return (f"M {c-3} {cy+0.6} L {c-3} {cy-3} L {c+0.6} {cy-3} Z "
                    f"M {c+3} {cy-0.6} L {c+3} {cy+3} L {c-0.6} {cy+3} Z")
        return (f"M {c-0.4} {cy-0.4} L {c-0.4} {cy-3.6} L {c-3.6} {cy-0.4} Z "     # restore: pointing in
                f"M {c+0.4} {cy+0.4} L {c+0.4} {cy+3.6} L {c+3.6} {cy+0.4} Z")
    filled = kind in ("maximize", "restore")

    def light_el(prefix, y, f, rm, show_symbol, darken=0.0):
        f = mix(f, (0, 0, 0), darken)
        rm = mix(rm, (0, 0, 0), darken)
        g = [f'<g id="{prefix}-center"><rect x="0" y="{y}" width="{BUTTON}" height="{BUTTON}" style="fill:none"/>',
             f'<circle cx="{c}" cy="{y + c}" r="{r - 0.25}" style="fill:{hexc(f)};stroke:{hexc(rm)};stroke-width:0.5"/>']
        if show_symbol:
            style = (f"fill:{hexc(sym)};fill-opacity:0.75;stroke:none" if filled
                     else f"fill:none;stroke:{hexc(sym)};stroke-opacity:0.75;stroke-width:1.2;stroke-linecap:round")
            g.append(f'<path d="{symbol(y + c)}" style="{style}"/>')
        g.append("</g>")
        return "\n".join(g)

    rows = [
        ("active", fill, rim, False, 0),
        ("hover", fill, rim, True, 0),
        ("pressed", fill, rim, True, 0.18),
        ("inactive", grey, grey_rim, False, 0),
        ("hover-inactive", fill, rim, True, 0),
        ("deactivated", grey, grey_rim, False, 0),
        ("deactivated-inactive", grey, grey_rim, False, 0),
    ]
    body = [light_el(pre, i * (BUTTON + 2), f, rm, s, d) for i, (pre, f, rm, s, d) in enumerate(rows)]
    return svg_doc(BUTTON, len(rows) * (BUTTON + 2), "\n".join(body) + "\n")


def aurorae_theme(p, name):
    d = OUT / "aurorae/themes" / name
    dark = p is DARK
    save_text(d / "metadata.desktop", "\n".join([
        "[Desktop Entry]",
        f"Name={p['name']}",
        f"Comment=jk_os's title bars ({'dark' if dark else 'light'}): buttons on the left, a translucent bar",
        "X-KDE-PluginInfo-Author=J.K. Robotics Pvt. Ltd.",
        f"X-KDE-PluginInfo-Name={name}",
        "X-KDE-PluginInfo-Version=1.0",
        "X-KDE-PluginInfo-License=GPLv2+",
        "X-KDE-PluginInfo-EnabledByDefault=true",
        "X-Plasma-API=javascript",
        "X-KDE-ServiceTypes=KWin/Decoration",
        "",
    ]))
    outline = "255,255,255,28" if dark else "0,0,0,40"
    outline_inactive = "255,255,255,18" if dark else "0,0,0,26"
    save_text(d / f"{name}rc", "\n".join([
        "# jk_os's title bars, generated by scripts/make-theme.py.",
        "[General]",
        f"ActiveTextColor={rgb(p['text'])}",
        f"InactiveTextColor={rgb(p['text_inactive'])}",
        "TitleAlignment=Center",
        "TitleVerticalAlignment=Center",
        "Animation=120",
        "ButtonGroupHover=true",
        f"ActiveOutlineColor={outline}",
        f"InactiveOutlineColor={outline_inactive}",
        "",
        "[Layout]",
        "BorderLeft=0", "BorderRight=0", "BorderBottom=0", "BorderTop=0",
        "TitleEdgeTop=6", "TitleEdgeBottom=6", "TitleEdgeLeft=12", "TitleEdgeRight=12",
        "TitleEdgeTopMaximized=6", "TitleEdgeBottomMaximized=6",
        "TitleEdgeLeftMaximized=12", "TitleEdgeRightMaximized=12",
        "TitleBorderLeft=12", "TitleBorderRight=12",
        "TitleHeight=22",
        f"ButtonWidth={BUTTON}", f"ButtonHeight={BUTTON}", "ButtonSpacing=6",
        "ButtonMarginTop=4", "ButtonMarginTopMaximized=4", "ExplicitButtonSpacer=8",
        f"PaddingLeft={PAD_L}", f"PaddingRight={PAD_R}", f"PaddingTop={PAD_T}", f"PaddingBottom={PAD_B}",
        f"CornerRadius={RADIUS}",
        "",
    ]))
    save_text(d / "decoration.svg", decoration_svg(p))
    for kind in ("close", "minimize", "maximize", "restore"):
        save_text(d / f"{kind}.svg", button_svg(kind, p))


# ---------------------------------------------------------------- Plasma style
GLASS = {       # Breeze's translucent parts, and how opaque jk_os makes them
    "translucent/widgets/panel-background.svg": 0.64,
    "translucent/dialogs/background.svg": 0.70,
    "translucent/widgets/tooltip.svg": 0.74,
}


def plasma_style():
    d = OUT / "plasma/desktoptheme/jk_os"
    save_text(d / "metadata.json", json.dumps({
        "KPlugin": {
            "Authors": [{"Name": "J.K. Robotics Pvt. Ltd."}],
            "Description": "jk_os: Breeze, with glass panels, pop-ups and tooltips (follows the colour scheme)",
            "EnabledByDefault": True,
            "Id": "jk_os",
            "License": "LGPL",
            "Name": "jk_os",
            "Version": "1.0",
        },
        "X-Plasma-API": "5.0",
    }, indent=4) + "\n")
    save_text(d / "plasmarc", "\n".join([
        "# jk_os's Plasma style (scripts/make-theme.py): Breeze for everything it",
        "# doesn't draw itself.",
        "[Settings]",
        "FallbackTheme=default",
        "",
        "[Wallpaper]",
        "defaultWallpaperTheme=jk_os",
        "defaultFileSuffix=.png",
        "defaultWidth=1920",
        "defaultHeight=1080",
        "",
        "# Panels turn opaque while a window is maximized.",
        "[AdaptiveTransparency]",
        "enabled=true",
        "",
        "[BlurBehindEffect]",
        "enabled=true",
        "",
        "# What shows through the glass: a little more contrast and colour, as",
        "# macOS's vibrancy does.",
        "[ContrastEffect]",
        "enabled=true",
        "contrast=0.35",
        "intensity=1.3",
        "saturation=1.8",
        "",
    ]))
    for rel, opacity in GLASS.items():
        src = (BREEZE_THEME / rel).read_text()
        n = 0

        def more_glass(m):
            nonlocal n
            n += 1
            return f"opacity:{opacity}"
        out = re.sub(r"opacity:0\.85", more_glass, src)
        assert n, rel
        save_text(d / rel, out)
    save_text(d / "widgets/tasks.svg", tasks_svg())


TASK_C, TASK_M = 16, 8          # task frame: corner size, stretched middle
COLOR_STYLE = """<style type="text/css" id="current-color-scheme">
.ColorScheme-Text { color:#232629; }
.ColorScheme-Highlight { color:#3daee9; }
.ColorScheme-NeutralText { color:#f67400; }
</style>"""


def tasks_svg():
    """The task manager's frames (widgets/tasks), as in macOS's dock: running
    applications get a small pill at the panel's edge (brighter for the
    active one), hovering lights up the icon's cell. Each state is a frame of
    nine parts, each part drawing only its own piece (no clipping: KSvg sizes
    a part by what it draws); a panel on another edge gets the pill on its
    side."""
    C, M = TASK_C, TASK_M
    S = C * 2 + M
    R, I = 6, 1                 # the cell's corner radius and inset
    T, E = 3, 2                 # the pill's thickness and distance from the edge
    states = {   # background opacity, pill opacity, pill colour class
        "normal": (0, 0.45, "ColorScheme-Text"),
        "focus": (0.07, 0.9, "ColorScheme-Text"),
        "hover": (0.13, 0, None),
        "attention": (0.07, 0.95, "ColorScheme-NeutralText"),
        "minimized": (0, 0.28, "ColorScheme-Text"),
        "progress": (0.22, 0, None),
    }
    edges = {"": "bottom", "north-": "top", "west-": "left", "east-": "right"}
    cols = {"left": (0, C), "mid": (C, M), "right": (C + M, C)}
    rows = {"top": (0, C), "mid": (C, M), "bottom": (C + M, C)}
    names = {("top", "left"): "topleft", ("top", "mid"): "top", ("top", "right"): "topright",
             ("mid", "left"): "left", ("mid", "mid"): "center", ("mid", "right"): "right",
             ("bottom", "left"): "bottomleft", ("bottom", "mid"): "bottom", ("bottom", "right"): "bottomright"}

    def cell_piece(row, col, y0):
        """The part of the rounded cell (x, y from I to S - I) inside a tile."""
        x, w = cols[col]
        y, h = rows[row]
        x0, x1 = max(x, I), min(x + w, S - I)
        ya, yb = max(y, I), min(y + h, S - I)
        ya, yb = ya + y0, yb + y0
        if row == "mid" or col == "mid":
            return f"M {x0} {ya} H {x1} V {yb} H {x0} Z"
        # a corner: round the outer one
        if (row, col) == ("top", "left"):
            return f"M {x0} {ya + R} A {R} {R} 0 0 1 {x0 + R} {ya} H {x1} V {yb} H {x0} Z"
        if (row, col) == ("top", "right"):
            return f"M {x0} {ya} H {x1 - R} A {R} {R} 0 0 1 {x1} {ya + R} V {yb} H {x0} Z"
        if (row, col) == ("bottom", "left"):
            return f"M {x0} {ya} H {x1} V {yb} H {x0 + R} A {R} {R} 0 0 1 {x0} {yb - R} Z"
        return f"M {x0} {ya} H {x1} V {yb - R} A {R} {R} 0 0 1 {x1 - R} {yb} H {x0} Z"

    def pill_piece(edge, row, col, y0):
        """The pill's piece in a tile: the middle part, or a round end."""
        r = T / 2
        if edge in ("bottom", "top"):
            if row != ("bottom" if edge == "bottom" else "top"):
                return None
            py = y0 + (S - E - T if edge == "bottom" else E)
            if col == "mid":
                return f"M {C} {py} H {C + M} V {py + T} H {C} Z"
            if col == "left":
                return f"M {C} {py} A {r} {r} 0 0 0 {C} {py + T} Z"
            return f"M {C + M} {py} A {r} {r} 0 0 1 {C + M} {py + T} Z"
        if col != ("right" if edge == "right" else "left"):
            return None
        px = S - E - T if edge == "right" else E
        if row == "mid":
            return f"M {px} {y0 + C} H {px + T} V {y0 + C + M} H {px} Z"
        if row == "top":
            return f"M {px} {y0 + C} A {r} {r} 0 0 1 {px + T} {y0 + C} Z"
        return f"M {px} {y0 + C + M} A {r} {r} 0 0 0 {px + T} {y0 + C + M} Z"

    out, y0 = [COLOR_STYLE], 0
    for pre, edge in edges.items():
        for state, (bg, pill, pill_class) in states.items():
            name = pre + state
            bg_class = "ColorScheme-Highlight" if state == "progress" else "ColorScheme-Text"
            for (row, col), part in names.items():
                x, w = cols[col]
                y, h = rows[row]
                g = [f'<g id="{name}-{part}">',
                     f'<rect x="{x}" y="{y0 + y}" width="{w}" height="{h}" style="fill:none;stroke:none"/>']
                if bg:
                    g.append(f'<path d="{cell_piece(row, col, y0)}" class="{bg_class}" '
                             f'style="fill:currentColor;fill-opacity:{bg};stroke:none"/>')
                d = pill_piece(edge, row, col, y0) if pill else None
                if d:
                    g.append(f'<path d="{d}" class="{pill_class}" '
                             f'style="fill:currentColor;fill-opacity:{pill};stroke:none"/>')
                g.append("</g>")
                out.append("".join(g))
            # the icon keeps its size: small content margins, not the parts' 16px
            for k, side in enumerate(("top", "bottom", "left", "right")):
                out.append(f'<rect id="{name}-hint-{side}-margin" x="{S + 4}" y="{y0 + 3 * k}" '
                           f'width="2" height="2" style="fill:none;stroke:none"/>')
            y0 += S + 4
    return svg_doc(S + 10, y0, "\n".join(out) + "\n")


def main():
    for p in (LIGHT, DARK):
        save_text(OUT / f"color-schemes/{p['id']}.colors", color_scheme(p))
    aurorae_theme(LIGHT, "jk_os-light")
    aurorae_theme(DARK, "jk_os-dark")
    plasma_style()


if __name__ == "__main__":
    main()
