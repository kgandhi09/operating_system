# Which GPU draws the desktop (jk-gui-session) and the dev session
# (jk-dev-session), sourced by both. On a laptop with an NVIDIA GPU next to
# the built-in one, the built-in GPU drives the laptop's screen and the NVIDIA
# one usually the HDMI port; the compositor renders on one of them and copies
# the picture to the other's screens.
#
# From ~/.config/jk_os/gpu or /etc/jk_os/gpu (shell syntax):
#   JK_GPU      the desktop (KWin):
#       nvidia   (default) the NVIDIA GPU, whenever NVIDIA's driver runs
#       auto     the NVIDIA GPU when a screen is connected to it at login,
#                otherwise the built-in one (longer battery life)
#       builtin  always the built-in GPU; programs can still use the NVIDIA
#                one with prime-run
#   JK_DEV_GPU  the dev session (cage), the same choices; default builtin:
#                cage (wlroots) copies a picture from the NVIDIA GPU to the
#                built-in one's screen only badly (the laptop's screen stops
#                updating), while the other way works, and a terminal needs
#                little GPU anyway
# Sets KWIN_DRM_DEVICES (KWin) and WLR_DRM_DEVICES (cage), primary first.

# jk_gpu_order <choice>: the DRM devices, primary first, for that choice.
jk_gpu_order() {
    use=builtin
    case "$1" in
        nvidia) use=nvidia ;;
        auto)
            for n in $nv; do
                for s in /sys/class/drm/${n##*/}-*/status; do
                    [ "$(cat "$s" 2>/dev/null)" = connected ] && use=nvidia
                done
            done ;;
    esac
    if [ "$use" = nvidia ] || [ -z "$others" ]; then
        echo $nv $others | tr ' ' ':'
    else
        echo $others $nv | tr ' ' ':'
    fi
}

jk_gpu_select() {
    JK_GPU=nvidia JK_DEV_GPU=builtin
    [ -f /etc/jk_os/gpu ] && . /etc/jk_os/gpu
    [ -f "${XDG_CONFIG_HOME:-$HOME/.config}/jk_os/gpu" ] && . "${XDG_CONFIG_HOME:-$HOME/.config}/jk_os/gpu"
    [ -d /sys/module/nvidia_drm ] || return 0
    nv="" others=""
    for c in /sys/class/drm/card[0-9] /sys/class/drm/card[0-9][0-9]; do
        [ -e "$c/device/vendor" ] || continue
        if [ "$(cat "$c/device/vendor")" = 0x10de ]; then nv="$nv /dev/dri/${c##*/}"
        else others="$others /dev/dri/${c##*/}"; fi
    done
    [ -n "$nv" ] || return 0
    KWIN_DRM_DEVICES=$(jk_gpu_order "$JK_GPU")
    WLR_DRM_DEVICES=$(jk_gpu_order "$JK_DEV_GPU")
    export KWIN_DRM_DEVICES WLR_DRM_DEVICES
}
