#!/usr/bin/env bash
# Maintainer tool: fetch NVIDIA's Linux driver (the .run package from
# download.nvidia.com, checked against its published SHA-256) and keep in
# userspace/nvidia/<arch> what jk_os uses from it. Builds never run this; they
# only use the tree in the repo, like the other sources.
#
#   scripts/update-nvidia.sh <version>        e.g. 595.104.02
#   NVIDIA_RUN=~/NVIDIA-Linux-x86_64-595.104.02.run scripts/update-nvidia.sh 595.104.02
#
# Kept (by the types in the package's .manifest):
#   kernel-open/     the open kernel modules' source (Turing and newer),
#                    built against jk_os's kernel by scripts/build-nvidia.sh
#   files/           the GSP firmware those modules load (files/firmware),
#                    and the libraries and tools: OpenGL/EGL/GLES (through
#                    GLVND), Vulkan (with ray tracing, OptiX), GBM (KWin,
#                    cage), EGL on Wayland and GBM, CUDA and OpenCL,
#                    NVENC/NVDEC, VDPAU, NVML and nvidia-smi
#   manifest         what goes where (read by build-nvidia.sh)
# Left out: 32-bit libraries, GLVND (jk_os builds its own), the X.org
# driver and nvidia-settings, the installer, and the closed kernel modules.
source "$(dirname "$0")/common.sh"
need curl sha256sum

ver="${1:-}"
[[ -n "$ver" ]] || die "usage: $0 <driver version> (see https://download.nvidia.com/XFree86/Linux-x86_64/latest.txt)"
case "$ARCH" in
    x86_64)  narch=x86_64 ;;
    aarch64) narch=aarch64 ;;
esac
run="NVIDIA-Linux-$narch-$ver.run"
url="https://download.nvidia.com/XFree86/Linux-$narch/$ver/$run"
DEST="$ROOT_DIR/userspace/nvidia/$ARCH"

tmp="$(mktemp -d "$ROOT_DIR/.update-nvidia.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

curl -fsSL "$url.sha256sum" -o "$tmp/sum" || die "no published checksum at $url.sha256sum"
if [[ -n "${NVIDIA_RUN:-}" ]]; then
    cp "$NVIDIA_RUN" "$tmp/$run"
else
    log "downloading $url"
    curl -fL --progress-bar --retry 5 --retry-all-errors -o "$tmp/$run" "$url"
fi
want=$(cut -d' ' -f1 "$tmp/sum")
have=$(sha256sum "$tmp/$run" | cut -d' ' -f1)
[[ "$want" == "$have" ]] || die "checksum mismatch for $run: $have, published $want"

log "unpacking"
sh "$tmp/$run" --extract-only --target "$tmp/x" >/dev/null
[[ -f "$tmp/x/.manifest" ]] || die "no .manifest in $run"

# The manifest: a header of 8 lines, then "<file> <mode> <type> ..." per file.
# Types kept; everything else (COMPAT32, XMODULE_*, installer, ...) is left out.
# GLVND's own libraries (libEGL, libGL, libGLX, libGLES*, libOpenGL,
# libGLdispatch) are left out: jk_os builds libglvnd itself.
keep_types='^(OPENGL_LIB|OPENGL_SYMLINK|GLVND_EGL_ICD_JSON|EGL_EXTERNAL_PLATFORM_JSON|VULKAN_ICD_JSON|GBM_BACKEND_LIB_SYMLINK|TLS_LIB|UTILITY_LIB|UTILITY_LIB_SYMLINK|UTILITY_BINARY|CUDA_LIB|CUDA_SYMLINK|OPENCL_LIB|OPENCL_LIB_SYMLINK|OPENCL_WRAPPER_LIB|OPENCL_WRAPPER_SYMLINK|CUDA_ICD|NVCUVID_LIB|NVCUVID_LIB_SYMLINK|ENCODEAPI_LIB|ENCODEAPI_LIB_SYMLINK|VDPAU_LIB|VDPAU_SYMLINK|FIRMWARE|APPLICATION_PROFILE)$'
# Parts for X.org, the installer, systemd, Vulkan SC and the Windows-style
# services (nvidia-powerd, NGX updater), by the package's MODULE: tag.
drop_modules='MODULE:(installer|xutils|nvlibpkcs11|vulkansc|nvtopps|pcc)$'
tail -n +9 "$tmp/x/.manifest" | awk -v keep="$keep_types" -v drop="$drop_modules" '
    $3 ~ keep && $0 !~ / COMPAT32 / && $NF !~ drop && $1 != "nvidia-ngx-updater" { print }' > "$tmp/manifest"
[[ -s "$tmp/manifest" ]] || die "nothing to keep from the manifest"

rm -rf "$DEST.new"; mkdir -p "$DEST.new/files"
while read -r file mode type rest; do
    [[ -e "$tmp/x/$file" || -L "$tmp/x/$file" ]] || continue
    mkdir -p "$DEST.new/files/$(dirname "$file")"
    cp -P "$tmp/x/$file" "$DEST.new/files/$file"
done < "$tmp/manifest"
cp "$tmp/manifest" "$DEST.new/manifest"
cp -a "$tmp/x/kernel-open" "$DEST.new/kernel-open"
cp "$tmp/x/LICENSE" "$DEST.new/LICENSE" 2>/dev/null || true
echo "$ver" > "$DEST.new/VERSION"
echo "$want  $run" > "$DEST.new/SHA256SUM"

rm -rf "$DEST"
mv "$DEST.new" "$DEST"
log "userspace/nvidia/$ARCH: NVIDIA $ver ($(du -sh "$DEST" | cut -f1))"
echo "Commit with: git add userspace/nvidia/$ARCH (large libraries go through Git LFS, see .gitattributes)"
