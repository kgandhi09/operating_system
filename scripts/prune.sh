#!/usr/bin/env bash
# Free disk space after a build (make PRESET=<preset> prune): delete the work
# directories of every package that finished building, for the preset's arch
# (build/<arch>/{net,tc,desktop}/<package>) and the build host's shared tools
# (build/host/<host arch>/<package>). What was built stays: the toolchains, the
# cross compiler, dyn/, the kernel build, rootfs and images.
#
# Nothing has to be rebuilt afterwards: a step decides from its stamp
# (<package>.done) alone, and starts from an empty directory whenever it does
# build again. A directory newer than its stamp (that package is being rebuilt
# right now) is left alone, and the shared host tools only when no other build
# is using them.
source "$(dirname "$0")/common.sh"
need flock

# prune_dir <dir>: delete <dir>/<package> for each <dir>/<package>.done
# older than the stamp, and print what that freed.
prune_dir() {
    local dir="$1" stamp work kb=0 n=0
    [[ -d "$dir" ]] || return 0
    for stamp in "$dir"/*.done; do
        work=${stamp%.done}
        [[ -d "$work" && "$stamp" -nt "$work" ]] || continue
        kb=$(( kb + $(du -sk "$work" | cut -f1) ))
        rm -rf "$work"
        n=$(( n + 1 ))
    done
    (( n == 0 )) || log "${dir#"$ROOT_DIR"/}: $n work directories, $(( kb / 1024 )) MB"
}

for w in net tc desktop; do
    prune_dir "$OUT_DIR/$w"
done

HOST_WORK="$ROOT_DIR/build/host/$HOST_ARCH"
if [[ -d "$HOST_WORK" ]]; then
    exec {host_lock}>"$HOST_WORK/.lock"
    if flock -n "$host_lock"; then
        prune_dir "$HOST_WORK"
    else
        warn "build/host/$HOST_ARCH is in use by a running build: left alone"
    fi
fi
log "build/: $(du -sh "$ROOT_DIR/build" | cut -f1), $(df -h --output=avail "$ROOT_DIR/build" | tail -1 | tr -d ' ') free"
