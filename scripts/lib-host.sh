#!/usr/bin/env bash
# The build machine: which systems may build jk_os, and what they must have.
# Sourced by common.sh (needs die and TARGETS_DIR), don't run it.
#
# A build machine is any Linux with the floor below (Ubuntu, Debian, ...), or
# an installed jk_os whose device category allows it (CATEGORY_BUILD_HOST=yes
# in its category.env: PCs, Jetson). jk_os on tablets and phones, and the
# live system (the installer USB stick), are refused. Everything beyond the
# floor the build makes itself (build-hosttools.sh): nothing comes from a
# package manager (docs/self-hosting.md).

# What every minimal Linux has, and all a build machine needs: a C and C++
# compiler, GNU make, bash and the basic file tools; git to have this repo.
HOST_FLOOR="gcc g++ make bash sed grep awk find xargs tar gzip xz bzip2 patch diff cmp flock git"

# check_build_host: stop on a system that may not build jk_os, or that lacks
# part of the floor.
check_build_host() {
    if [[ -f /etc/jk_os/target ]]; then
        local category mode allowed
        category=$(sed -n 's/^JK_CATEGORY=//p' /etc/jk_os/target)
        mode=$(cat /run/jk-mode 2>/dev/null || echo unknown)
        [[ "$mode" == installed ]] \
            || die "this is jk_os's $mode system: build jk_os on an installed jk_os (docs/self-hosting.md)"
        # The category's own setting, read apart (load_category is the target's).
        allowed=$(CATEGORY_BUILD_HOST=; source "$TARGETS_DIR/devices/$category/category.env" 2>/dev/null
                  echo "$CATEGORY_BUILD_HOST")
        [[ "$allowed" == yes ]] \
            || die "jk_os on a ${category:-unknown} device doesn't build jk_os: use a jk_os PC or Jetson, or another Linux (docs/self-hosting.md)"
    fi
    local c missing=()
    for c in $HOST_FLOOR; do
        command -v "$c" >/dev/null 2>&1 || missing+=("$c")
    done
    (( ${#missing[@]} == 0 )) \
        || die "the build machine lacks: ${missing[*]} (the floor every build machine needs; on Ubuntu: sudo apt install build-essential git git-lfs)"
}
