# Which GPU draws the desktop (jk-gui-session) and the dev session
# (jk-dev-session), sourced by both. On a laptop with an NVIDIA GPU next to
# the built-in one, the built-in GPU drives the laptop's screen and the NVIDIA
# one usually the HDMI port; the compositor renders on one of them and copies
# the picture to the other's screens.
#
# JK_GPU, from ~/.config/jk_os/gpu or /etc/jk_os/gpu (shell syntax):
#   auto     (default) the NVIDIA GPU when a screen is connected to it,
#            otherwise the built-in one (longer battery life)
#   nvidia   always the NVIDIA GPU (NVIDIA's driver)
#   builtin  always the built-in GPU; programs can still use the NVIDIA one
#            with prime-run
# Sets KWIN_DRM_DEVICES (KWin) and WLR_DRM_DEVICES (cage), primary first.

jk_gpu_select() {
    JK_GPU=auto
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
    use=builtin
    case "$JK_GPU" in
        nvidia) use=nvidia ;;
        auto)
            for n in $nv; do
                for s in /sys/class/drm/${n##*/}-*/status; do
                    [ "$(cat "$s" 2>/dev/null)" = connected ] && use=nvidia
                done
            done ;;
    esac
    [ "$use" = nvidia ] || return 0
    list=$(echo $nv $others | tr ' ' ':')
    export KWIN_DRM_DEVICES="$list" WLR_DRM_DEVICES="$list"
}
