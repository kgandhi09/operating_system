#!/usr/bin/env bash
# Choose what jk_os is built for and save it in build.conf (make config):
#
#   1. device category   targets/devices/<category>   (pc, tablet, robotics-hpc, ...)
#   2. device            targets/devices/<category>/<device>
#   3. kernel            targets/kernels/<kernel>, the ones the device uses
#   4. arch              the ones both the device and the kernel support
#   5. name              free, for the output file name (e.g. asus-laptop)
#
# Each menu offers only what fits the choices before it (arrow keys or j/k to
# move, Enter to pick), and starts on the current build.conf's choice.
#
#   scripts/configure.sh          choose, then save
#   scripts/configure.sh --show   print the current target
#   scripts/configure.sh --check  fail unless jk_os can build it (make runs
#                                 this before any build step)
JK_NO_TARGET=1
source "$(dirname "$0")/common.sh"

case "${1:-}" in
--show)
    load_target
    echo "build target ($( [[ -f "$BUILD_CONF" ]] && echo "$BUILD_CONF" || echo "no build.conf: default" )):"
    print_target
    exit 0 ;;
--check)
    load_target
    in_list "$DEVICE_BOOT" "$BOOT_FORMATS" \
        || die "device '$JK_DEVICE' boots from '$DEVICE_BOOT', which jk_os can't build yet (only: $BOOT_FORMATS)"
    exit 0 ;;
esac
[[ $# -eq 0 ]] || die "usage: $0 [--show | --check]"
[[ -t 0 ]] || die "make config asks questions: run it in a terminal (or write build.conf yourself)"

# Defaults: the current build.conf, if it still makes sense.
JK_CATEGORY= JK_DEVICE= JK_KERNEL= JK_ARCH= JK_NAME=
# shellcheck source=/dev/null
[[ -f "$BUILD_CONF" ]] && source "$BUILD_CONF"
cur_category=$JK_CATEGORY cur_device=$JK_DEVICE cur_kernel=$JK_KERNEL cur_arch=$JK_ARCH cur_name=$JK_NAME

# choose <title> <default> <value>|<description> ...: a menu moved with the
# arrow keys (or j/k) that starts on the default; Enter picks. Sets CHOICE.
# A single choice is taken as is.
choose() {
    local title=$1 def=$2 i n cur=0 key rest line cols
    shift 2
    local values=() descs=()
    for i in "$@"; do values+=("${i%%|*}"); descs+=("${i#*|}"); done
    n=${#values[@]}
    (( n )) || die "no $title to choose from"
    for (( i = 0; i < n; i++ )); do
        [[ "${values[i]}" == "$def" ]] && cur=$i
    done
    echo
    if (( n == 1 )); then
        CHOICE=${values[0]}
        printf '%s: \e[1m%s\e[0m (the only choice)\n' "$title" "$CHOICE"
        return
    fi
    echo "$title:"
    # Lines are cut to the terminal's width: a wrapped one would throw off
    # the redraw, which moves the cursor back up one line per choice.
    cols=$(tput cols 2>/dev/null || echo 80)
    printf '\e[?25l'
    while :; do
        for (( i = 0; i < n; i++ )); do
            printf -v line '%-18s %s' "${values[i]}" "${descs[i]}"
            line=${line:0:cols-5}
            if (( i == cur )); then printf '\r\e[2K  \e[7m> %s\e[0m\n' "$line"
            else printf '\r\e[2K    %s\n' "$line"; fi
        done
        IFS= read -rsn1 key || die "cancelled"
        if [[ "$key" == $'\e' ]]; then
            IFS= read -rsn2 -t 0.1 rest || true
            case "$rest" in
                '[A'|OA) key=k ;;
                '[B'|OB) key=j ;;
            esac
        fi
        case "$key" in
            k) cur=$(( (cur + n - 1) % n )) ;;
            j) cur=$(( (cur + 1) % n )) ;;
            '') break ;;
        esac
        printf '\e[%dA' "$n"
    done
    printf '\e[?25h'
    CHOICE=${values[cur]}
    # Replace the menu with the choice.
    printf '\e[%dA\r\e[J%s: \e[1m%s\e[0m\n' $(( n + 1 )) "$title" "$CHOICE"
}

# The menus hide the cursor; show it again however this ends.
trap 'printf "\e[?25h"' EXIT
echo "jk_os build target: up/down (or j/k) to move, Enter to choose, Ctrl-C quits without saving"

# 1. Category.
items=()
for c in $(target_categories); do
    load_category "$c"
    items+=("$c|$CATEGORY_DESC")
done
choose "Device category" "$cur_category" "${items[@]}"
JK_CATEGORY=$CHOICE

# 2. Device.
items=()
for d in $(target_devices "$JK_CATEGORY"); do
    load_device "$JK_CATEGORY" "$d"
    items+=("$d|$DEVICE_DESC${DEVICE_STATUS:+ [$DEVICE_STATUS]}")
done
choose "Device" "$cur_device" "${items[@]}"
JK_DEVICE=$CHOICE
load_device "$JK_CATEGORY" "$JK_DEVICE"
device_kernels=$DEVICE_KERNELS

# 3. Kernel: those the device uses, that exist, and share an arch with it.
items=()
for k in $device_kernels; do
    load_kernel "$k" || { warn "device $JK_DEVICE lists kernel '$k', but targets/kernels/$k has no kernel.env"; continue; }
    [[ -n "$(target_archs)" ]] || continue
    items+=("$k|$KERNEL_DESC")
done
choose "Kernel" "$cur_kernel" "${items[@]}"
JK_KERNEL=$CHOICE
load_kernel "$JK_KERNEL"

# 4. Arch. Default: the current one, else the host's.
items=()
for a in $(target_archs); do
    if [[ "$a" == "$HOST_ARCH" ]]; then items+=("$a|native build on this machine")
    else items+=("$a|cross-compiled (${a}-linux-gnu-gcc)"); fi
done
def_arch=$cur_arch
in_list "$def_arch" "$(target_archs)" || def_arch=$HOST_ARCH
choose "Architecture" "$def_arch" "${items[@]}"
JK_ARCH=$CHOICE

# 5. Name. Default: the current one when the device didn't change, else the
# device's own name.
def_name=$JK_DEVICE
[[ -n "$cur_name" && "$cur_device" == "$JK_DEVICE" ]] && def_name=$cur_name
echo
echo "Name for this build, used in the output file name (e.g. asus-laptop):"
while :; do
    read -r -p "  name [$def_name]: " JK_NAME || die "cancelled"
    JK_NAME=${JK_NAME:-$def_name}
    valid_name "$JK_NAME" && break
    echo "  use lowercase letters, digits, '.', '_' and '-' (starting with a letter or digit)"
done

err=$(check_target) || die "$err"
check_target
echo
echo "build target:"
print_target
in_list "$DEVICE_BOOT" "$BOOT_FORMATS" \
    || warn "this device boots from '$DEVICE_BOOT', which jk_os can't build yet: make will refuse it"
choose "Save to $BUILD_CONF" yes "yes|save, then build with make" "no|quit without saving"
[[ "$CHOICE" == yes ]] || { echo "not saved"; exit 1; }

cat > "$BUILD_CONF" <<EOF
# jk_os build target, written by make config (scripts/configure.sh).
JK_CATEGORY=$JK_CATEGORY
JK_DEVICE=$JK_DEVICE
JK_KERNEL=$JK_KERNEL
JK_ARCH=$JK_ARCH
JK_NAME=$JK_NAME
EOF
log "saved $BUILD_CONF: run make to build"
