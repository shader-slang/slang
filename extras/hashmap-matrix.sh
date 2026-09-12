#!/usr/bin/env bash
#
# Configure and build Slang once per (hash function, hash map) combination so
# that the alternatives selected by SLANG_HASH and SLANG_HASHMAP can be checked
# for compilability and benchmarked against each other. See the "Hash map and
# hash function selection" section of docs/building.md.
#
# Every combination is configured in parallel and then built in parallel, with
# the available cores divided between the builds, so that a combination which
# fails to compile shows up early rather than after every preceding build has
# finished.
#
# Each combination gets its own build directory, build-<hash>-<map>, in the
# repository root, so that runs are incremental and the ordinary ./build
# directory is left alone. Directories are never deleted; remove them yourself
# when you are done, they are large.
#
# Usage:
#   extras/hashmap-matrix.sh                 # one build per map and per hash
#   extras/hashmap-matrix.sh --all           # the full cross product
#   extras/hashmap-matrix.sh ABSL_FLAT/BOOST TSL_ROBIN/WYHASH
#
# A combination is written MAP/HASH. Each line of output is one result.

set -u

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$repo_root" || exit 1

maps=(
    UNORDERED_DENSE
    BOOST_FLAT
    BOOST_NODE
    BOOST_UNORDERED
    ABSL_FLAT
    ABSL_NODE
    TSL_ROBIN
    STD
)
hashes=(WYHASH BOOST ABSL STD)
default_map=UNORDERED_DENSE
default_hash=WYHASH

# The targets worth building to prove a combination compiles. Everything that
# uses Slang::Dictionary ends up in one of these two, and slang-rt compiles
# source/core a second time with its own export macros.
targets=(slang slang-rt)

# A nix devShell exports the flags this checkout is normally configured with
# (system LLVM, system DXC, ...). Honour them so these builds match the
# developer's ordinary build, and let them override the defaults below.
nix_flags=()
if [ -n "${cmakeFlags:-}" ]; then
    read -ra nix_flags <<<"$cmakeFlags"
fi

# Pick the combinations to build.
combos=()
case "${1:-}" in
"")
    for map in "${maps[@]}"; do
        combos+=("$map/$default_hash")
    done
    for hash in "${hashes[@]}"; do
        [ "$hash" = "$default_hash" ] && continue
        combos+=("$default_map/$hash")
    done
    ;;
--all)
    for map in "${maps[@]}"; do
        for hash in "${hashes[@]}"; do
            combos+=("$map/$hash")
        done
    done
    ;;
*)
    combos=("$@")
    ;;
esac

# Share the machine between the concurrent builds.
total_cores=$(nproc 2>/dev/null || echo 4)
cores_each=$((total_cores / ${#combos[@]}))
[ "$cores_each" -lt 1 ] && cores_each=1

build_dir_for() {
    printf 'build-%s' "$(printf '%s-%s' "$2" "$1" | tr '[:upper:]' '[:lower:]')"
}

configure_one() {
    local map="$1" hash="$2"
    local dir
    dir=$(build_dir_for "$map" "$hash")
    local log="$dir/hashmap-matrix.log"

    mkdir -p "$dir" || return 1
    : >"$log"

    if cmake -S . -B "$dir" -G "Ninja Multi-Config" \
        -DSLANG_ENABLE_TESTS=OFF \
        -DSLANG_ENABLE_EXAMPLES=OFF \
        -DSLANG_ENABLE_GFX=OFF \
        -DSLANG_ENABLE_REPLAYER=OFF \
        -DSLANG_ENABLE_PCH=OFF \
        "${nix_flags[@]}" \
        -DSLANG_HASHMAP="$map" \
        -DSLANG_HASH="$hash" \
        >>"$log" 2>&1; then
        return 0
    fi
    echo "$map/$hash: CONFIGURE FAILED ($log)"
    return 1
}

build_one() {
    local map="$1" hash="$2"
    local dir
    dir=$(build_dir_for "$map" "$hash")
    local log="$dir/hashmap-matrix.log"

    if cmake --build "$dir" --config Debug --parallel "$cores_each" \
        --target "${targets[@]}" >>"$log" 2>&1; then
        echo "$map/$hash: OK ($dir)"
        return 0
    fi
    {
        echo "$map/$hash: BUILD FAILED ($log)"
        grep -E "error:" "$log" | sort -u | head -20
    }
    return 1
}

# Configure everything first: a configure failure is cheap to hit and there is
# no point starting long builds for the combinations that did succeed until we
# know which ones those are.
echo "Configuring ${#combos[@]} combinations..."
configured=()
pids=()
for combo in "${combos[@]}"; do
    IFS=/ read -r map hash <<<"$combo"
    configure_one "$map" "$hash" &
    pids+=("$!")
done
for i in "${!pids[@]}"; do
    if wait "${pids[$i]}"; then
        configured+=("${combos[$i]}")
    fi
done

status=0
[ "${#configured[@]}" -ne "${#combos[@]}" ] && status=1

if [ "${#configured[@]}" -eq 0 ]; then
    exit 1
fi

echo "Building ${#configured[@]} combinations, $cores_each core(s) each..."
pids=()
for combo in "${configured[@]}"; do
    IFS=/ read -r map hash <<<"$combo"
    build_one "$map" "$hash" &
    pids+=("$!")
done
for pid in "${pids[@]}"; do
    wait "$pid" || status=1
done

exit $status
