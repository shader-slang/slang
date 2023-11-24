#!/usr/bin/env bash

set -e

help() {
  me=$(basename "$0")
  cat <<EOF
$me: Fetch, build and install LLVM for Slang

Options:
  --source-url: Where to find the source tar.xz archive, default: $source_url
  --source-dir: Unpack and build in this directory: default $source_dir
  --config: The configuration to build, default $config
  --install-prefix: Install under this prefix
  --: Any following arguments will be passed to the CMake configuration command
EOF
}

#
# Some helper functions
#
msg(){
  printf "%s\n" "$1" >&2
}

fail() {
  msg "$1"
  exit 1
}

for prog in "curl" "cmake" "ninja" "tar"; do
  if ! command -v "$prog" &>/dev/null; then
    msg "This script needs $prog, but it isn't in \$PATH"
    missing_bin=1
  fi
done
if [ "$missing_bin" ]; then
  exit 1
fi

#
# Temp dir with cleanup on exit
#
temp_dir=$(mktemp -d)
cleanup() {
  local exit_status=$?
  rm -rf "$temp_dir"
  exit $exit_status
}
trap cleanup EXIT SIGHUP SIGINT SIGTERM

#
# Options and parsing
#
repo_url=https://github.com/llvm/llvm-project
version=13.0.1
source_url=$repo_url/releases/download/llvmorg-$version/llvm-project-$version.src.tar.xz
source_dir=$temp_dir
install_prefix=
config=Release
extra_arguments=()

while [[ "$#" -gt 0 ]]; do
  case $1 in
  -h | --help) help; exit ;;
  --source-url) source_url=$2; shift;;
  --source-dir) source_dir=$2; shift;;
  --config) config=$2; shift;;
  --install-prefix) install_prefix=$2; shift;;
  --) shift; extra_arguments+=("$@"); break;;
  *)
    msg "Unknown parameter passed: $1"
    help >&2
    exit 1
    ;;
  esac
  shift
done

[ -n "$source_url" ] || fail "please set --source-url"
[ -n "$source_dir" ] || fail "please set --source-dir"
[ -n "$config" ] || fail "please set --config"
[ -n "$install_prefix" ] || fail "please set --install-prefix"

msg "##########################################################"
msg "# Fetching LLVM from $source_url"
msg "##########################################################"
mkdir -p "$source_dir"
curl -L "$source_url" | tar xJ --strip-components 1 -C "$source_dir"

msg "##########################################################"
msg "# Configuring LLVM in $source_dir"
msg "##########################################################"
cmake_arguments_for_slang=(
  # Don't build unnecessary things
  -DLLVM_BUILD_LLVM_C_DYLIB=0
  -DLLVM_INCLUDE_BENCHMARKS=0
  -DLLVM_INCLUDE_DOCS=0
  -DLLVM_INCLUDE_EXAMPLES=0
  -DLLVM_INCLUDE_TESTS=0
  -DLLVM_ENABLE_TERMINFO=0
  -DCLANG_BUILD_TOOLS=0
  -DCLANG_ENABLE_STATIC_ANALYZER=0
  -DCLANG_ENABLE_ARCMT=0
  -DCLANG_INCLUDE_DOCS=0
  -DCLANG_INCLUDE_TESTS=0
  # Requirements for Slang
  -DLLVM_ENABLE_PROJECTS=clang
  "-DLLVM_TARGETS_TO_BUILD=X86;ARM;AArch64"
  -DLLVM_BUILD_TOOLS=1
)
build_dir=$source_dir/build
mkdir -p "$build_dir"
cmake \
  -S "$source_dir/llvm" -B "$build_dir" \
  -G "Ninja Multi-Config" \
  "${cmake_arguments_for_slang[@]}" \
  "${extra_arguments[@]}"

msg "##########################################################"
msg "# Building LLVM in $build_dir"
msg "##########################################################"
cmake --build "$build_dir" -j --config "$config"

msg "##########################################################"
msg "# Installing LLVM to $install_prefix"
msg "##########################################################"
cmake --install "$build_dir" --prefix "$install_prefix" --config "$config"

msg "##########################################################"
msg "LLVM installed in $install_prefix"
msg "Please add $install_prefix to CMAKE_PREFIX_PATH"
msg "##########################################################"
