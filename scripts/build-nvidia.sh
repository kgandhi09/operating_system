#!/usr/bin/env bash
# NVIDIA's driver for jk_os (userspace/nvidia/<arch>, see update-nvidia.sh):
# its open kernel modules built against jk_os's kernel, and its libraries,
# tools, GSP firmware and configuration files laid out as installed, in
# build/<arch>/nvidia. build-rootfs.sh copies that tree into the image;
# /etc/init.d/S11gpu loads the modules when an NVIDIA GPU is present.
source "$(dirname "$0")/common.sh"

NV_SRC="$ROOT_DIR/userspace/nvidia/$ARCH"
NV_OUT="$OUT_DIR/nvidia"
if [[ ! -f "$NV_SRC/VERSION" ]]; then
    warn "no userspace/nvidia/$ARCH: the image gets no NVIDIA driver (scripts/update-nvidia.sh adds one)"
    rm -rf "$NV_OUT"
    exit 0
fi
ver=$(cat "$NV_SRC/VERSION")
[[ -f "$KERNEL_OUT/$KIMAGE" ]] || die "kernel not built yet (run: make kernel)"
grep -q '^CONFIG_MODULES=y' "$KERNEL_OUT/.config" || die "the kernel has no module support (CONFIG_MODULES)"
krel=$(cat "$KERNEL_OUT/include/config/kernel.release")

# Rebuilt when the kernel or the driver changes.
stamp="$NV_OUT.done"
want="$ver $krel $(stat -c %Y "$KERNEL_OUT/$KIMAGE") $(cat "$0" | sha256sum | cut -c1-16)"
if [[ -f "$stamp" && "$(cat "$stamp")" == "$want" ]]; then
    log "NVIDIA $ver: up to date"
    exit 0
fi
rm -rf "$NV_OUT" "$stamp"

# ---------------------------------------------------------------- kernel modules
log "building NVIDIA $ver kernel modules for linux $krel ($ARCH)"
work="$OUT_DIR/nvidia-kernel"
rm -rf "$work"
cp -a "$NV_SRC/kernel-open" "$work"
make -C "$work" -j"$JOBS" modules SYSSRC="$KERNEL_SRC" SYSOUT="$KERNEL_OUT" \
    ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" CC="${CROSS_COMPILE}gcc" LD="${CROSS_COMPILE}ld" \
    > "$work.log" 2>&1 || { tail -n 40 "$work.log" >&2; die "NVIDIA kernel modules failed, see $work.log"; }
moddir="$NV_OUT/usr/lib/modules/$krel/extra"
mkdir -p "$moddir"
for m in nvidia nvidia-modeset nvidia-drm nvidia-uvm nvidia-peermem; do
    [[ -f "$work/$m.ko" ]] || continue
    install -m 0644 "$work/$m.ko" "$moddir/$m.ko"
    "${CROSS_COMPILE}strip" --strip-debug "$moddir/$m.ko"
done
[[ -f "$moddir/nvidia.ko" ]] || die "no nvidia.ko was built"

# ---------------------------------------------------------------- libraries, tools
# The manifest lines: <file> <mode> <type> [NATIVE] [<subdir>/] [<link target>] MODULE:<name>
log "installing NVIDIA $ver libraries and tools"
F="$NV_SRC/files"
L="$NV_OUT/usr/lib"
mkdir -p "$L" "$NV_OUT/usr/bin"
while read -r file mode type rest; do
    set -- $rest
    [[ "${1:-}" == NATIVE ]] && shift
    sub=""
    if [[ "${1:-}" == */ ]]; then
        [[ "$1" != / ]] && sub="$1"
        shift
    fi
    target=""
    [[ "${1:-}" != MODULE:* ]] && target="${1:-}"
    base="${file##*/}"
    case "$type" in
        *_SYMLINK|GBM_BACKEND_LIB_SYMLINK)
            case "$type" in
                GBM_BACKEND_LIB_SYMLINK) dir="$L/gbm"; target="../$target" ;;
                UTILITY_BIN_SYMLINK) dir="$NV_OUT/usr/bin" ;;
                *) dir="$L/$sub" ;;
            esac
            mkdir -p "$dir"
            ln -sfn "$target" "$dir/$base" ;;
        OPENGL_LIB|TLS_LIB|UTILITY_LIB|CUDA_LIB|OPENCL_LIB|OPENCL_WRAPPER_LIB|NVCUVID_LIB|ENCODEAPI_LIB|VDPAU_LIB)
            install -D -m 0755 "$F/$file" "$L/$sub$base" ;;
        UTILITY_BINARY)
            install -D -m 0755 "$F/$file" "$NV_OUT/usr/bin/$base" ;;
        FIRMWARE)
            install -D -m 0444 "$F/$file" "$L/firmware/nvidia/$ver/$base" ;;
        GLVND_EGL_ICD_JSON)
            install -D -m 0644 "$F/$file" "$NV_OUT/usr/share/glvnd/egl_vendor.d/$base" ;;
        EGL_EXTERNAL_PLATFORM_JSON)
            install -D -m 0644 "$F/$file" "$NV_OUT/usr/share/egl/egl_external_platform.d/$base" ;;
        VULKAN_ICD_JSON)
            install -D -m 0644 "$F/$file" "$NV_OUT/usr/share/vulkan/$sub$base" ;;
        CUDA_ICD)
            install -D -m 0644 "$F/$file" "$NV_OUT/etc/OpenCL/vendors/$base" ;;
        APPLICATION_PROFILE)
            install -D -m 0644 "$F/$file" "$NV_OUT/usr/share/nvidia/$base" ;;
        *) die "manifest: don't know where $type ($file) goes" ;;
    esac
done < "$NV_SRC/manifest"
# The links ldconfig would make: each library under its SONAME (EGL's
# platform libraries are asked for by it, libnvidia-egl-wayland.so.1 ...).
find "$L" -maxdepth 2 -type f -name '*.so*' | while read -r lib; do
    so=$("${CROSS_COMPILE}readelf" -d "$lib" 2>/dev/null | sed -n 's/.*(SONAME).*\[\(.*\)\]/\1/p')
    [[ -n "$so" && "$so" != "${lib##*/}" && ! -e "$(dirname "$lib")/$so" ]] && ln -s "${lib##*/}" "$(dirname "$lib")/$so"
    true
done
install -D -m 0644 "$NV_SRC/LICENSE" "$NV_OUT/usr/share/doc/nvidia/LICENSE"

echo "$want" > "$stamp"
log "NVIDIA $ver: $(du -sh "$NV_OUT" | cut -f1) in $NV_OUT"
