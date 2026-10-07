#!/usr/bin/env python3
"""Convert the supplied particle MP4 into a Qt AnimatedImage asset.

Run on the build host after changing jk_os_particle_splash.mp4. The generated
WebP is committed, so building jk_os does not need GStreamer or Pillow.
"""

from pathlib import Path
from subprocess import run
from tempfile import TemporaryDirectory

from PIL import Image


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "jk_os_particle_splash.mp4"
OUTPUT = (ROOT / "rootfs/usr/share/plasma/look-and-feel/org.jk_os.desktop/"
          "contents/splash/particle-splash.webp")


def main():
    with TemporaryDirectory(prefix="jk-splash-") as directory:
        frames = Path(directory)
        run([
            "gst-launch-1.0", "-q", "filesrc", f"location={SOURCE}", "!",
            "qtdemux", "!", "h264parse", "!", "avdec_h264", "!",
            "videorate", "!", "video/x-raw,framerate=30/1", "!",
            "videoscale", "!", "video/x-raw,width=1280,height=720", "!",
            "videoconvert", "!", "pngenc", "!", "multifilesink",
            f"location={frames}/frame-%03d.png",
        ], check=True)
        pictures = [Image.open(path).convert("RGB") for path in sorted(frames.glob("*.png"))]
        if len(pictures) != 180:
            raise RuntimeError(f"Expected 180 frames, got {len(pictures)}")
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        pictures[0].save(
            OUTPUT, format="WEBP", save_all=True, append_images=pictures[1:],
            duration=[33, 33, 34] * 60, loop=1, quality=85, method=4,
        )
        print(f"Wrote {OUTPUT} ({OUTPUT.stat().st_size:,} bytes)")


if __name__ == "__main__":
    main()
