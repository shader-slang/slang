#!/usr/bin/env bash
#
# compile-sascha-willems-shaders.sh - Compile Slang shaders from SaschaWillems/Vulkan
#
# This script compiles all Slang shaders from Sascha Willems's Vulkan examples
# repository (https://github.com/SaschaWillems/Vulkan) to detect regressions
# in the Slang compiler. It replicates the CI workflow for local debugging.
#
# Prerequisites: Slang must already be built (Release or Debug) at build/<config>/bin/
#
# Usage:
#   ./extras/compile-sascha-willems-shaders.sh [options]
#
# Options:
#   --slangc PATH  Path to slangc executable (default: auto-detect from build/)
#   --repo DIR     Path to existing SaschaWillems/Vulkan clone (default: clones to tmp/)
#   --clean        Remove existing clone and start fresh
#   --spirv-val    Enable SPIRV validation (sets SLANG_RUN_SPIRV_VALIDATION=1)
#   --help         Show this help message
#

set -u

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Shaders with known compilation issues in the Slang compiler.
# Each entry is a relative path under shaders/slang/.
# These are skipped during compilation and tracked separately.
# Remove entries as the underlying issues are fixed.
KNOWN_FAILURES=(
  # SV_PointSize cannot be used as input in fragment stage when
  # vertex/fragment share a struct (https://github.com/shader-slang/slang/issues/10473)
  "computenbody/particle.slang"
  "computeparticles/particle.slang"
  "particlesystem/particle.slang"
)

CLEAN=false
SPIRV_VAL=false
SLANGC=""
REPO_DIR=""

while [[ $# -gt 0 ]]; do
  case $1 in
  --slangc)
    if [[ $# -lt 2 ]]; then
      echo -e "${RED}Missing value for --slangc${NC}"
      exit 1
    fi
    SLANGC="$2"
    shift 2
    ;;
  --repo)
    if [[ $# -lt 2 ]]; then
      echo -e "${RED}Missing value for --repo${NC}"
      exit 1
    fi
    REPO_DIR="$2"
    shift 2
    ;;
  --clean)
    CLEAN=true
    shift
    ;;
  --spirv-val)
    SPIRV_VAL=true
    shift
    ;;
  --help)
    head -n 22 "$0" | grep "^#" | sed 's/^# //' | sed 's/^#//'
    exit 0
    ;;
  *)
    echo -e "${RED}Unknown option: $1${NC}"
    echo "Use --help for usage information"
    exit 1
    ;;
  esac
done

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[OK]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[FAIL]${NC} $1"; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SLANG_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$SLANG_ROOT" || exit 1

if [ ! -f "CMakeLists.txt" ]; then
  log_error "Cannot find Slang repository root"
  exit 1
fi

# Find slangc
if [ -z "$SLANGC" ]; then
  for candidate in \
    "$SLANG_ROOT/build/Release/bin/slangc.exe" \
    "$SLANG_ROOT/build/Release/bin/slangc" \
    "$SLANG_ROOT/build/Debug/bin/slangc.exe" \
    "$SLANG_ROOT/build/Debug/bin/slangc"; do
    if [ -f "$candidate" ]; then
      SLANGC="$candidate"
      break
    fi
  done
fi

if [ -z "$SLANGC" ] || [ ! -x "$SLANGC" ]; then
  log_error "slangc not found or not executable. Build Slang first or pass --slangc PATH"
  exit 1
fi

log_info "Using slangc: $SLANGC"
if ! "$SLANGC" -version; then
  log_error "Failed to execute slangc at $SLANGC"
  exit 1
fi

# Clone or use existing repo
if [ -z "$REPO_DIR" ]; then
  REPO_DIR="$SLANG_ROOT/tmp/SaschaWillems-Vulkan"
fi

if [ "$CLEAN" = true ] && [ -d "$REPO_DIR" ]; then
  log_warning "Removing existing clone at $REPO_DIR..."
  rm -rf "$REPO_DIR"
fi

if [ ! -d "$REPO_DIR" ]; then
  log_info "Cloning SaschaWillems/Vulkan (shallow)..."
  if ! git clone --depth 1 https://github.com/SaschaWillems/Vulkan.git "$REPO_DIR"; then
    log_error "Failed to clone SaschaWillems/Vulkan into $REPO_DIR"
    exit 1
  fi
  log_success "Cloned to $REPO_DIR"
else
  log_info "Using existing clone at $REPO_DIR"
fi

SHADER_DIR="$REPO_DIR/shaders/slang"
if [ ! -d "$SHADER_DIR" ]; then
  log_error "Shader directory not found: $SHADER_DIR"
  exit 1
fi

if [ "$SPIRV_VAL" = true ]; then
  export SLANG_RUN_SPIRV_VALIDATION=1
  log_info "SPIRV validation enabled"
fi

# Create temp directory for output
OUTDIR=$(mktemp -d)
trap 'rm -rf "$OUTDIR"' EXIT

is_known_failure() {
  local rel_path="$1"
  for known in "${KNOWN_FAILURES[@]}"; do
    if [ "$rel_path" = "$known" ]; then
      return 0
    fi
  done
  return 1
}

log_info "Compiling shaders from $SHADER_DIR..."
echo ""

success=0
fail=0
skip=0
skip_known=0
spv_count=0
fail_details=""

get_output_ext() {
  case "$1" in
  vertex) echo ".vert" ;;
  fragment) echo ".frag" ;;
  compute) echo ".comp" ;;
  raygeneration) echo ".rgen" ;;
  miss) echo ".rmiss" ;;
  closesthit) echo ".rchit" ;;
  callable) echo ".rcall" ;;
  intersection) echo ".rint" ;;
  anyhit) echo ".rahit" ;;
  mesh) echo ".mesh" ;;
  amplification) echo ".task" ;;
  geometry) echo ".geom" ;;
  hull) echo ".tesc" ;;
  domain) echo ".tese" ;;
  esac
}

ALL_STAGES="vertex fragment compute raygeneration miss closesthit callable intersection anyhit mesh amplification geometry hull domain"

while IFS= read -r slang_file; do
  content=$(cat "$slang_file")
  stages=""
  for stage in $ALL_STAGES; do
    if echo "$content" | grep -q "\[shader(\"$stage\")\]"; then
      stages="$stages $stage"
    fi
  done
  stages=$(echo "$stages" | xargs)

  rel_path="${slang_file#"$SHADER_DIR"/}"

  if [ -z "$stages" ]; then
    skip=$((skip + 1))
    continue
  fi

  if is_known_failure "$rel_path"; then
    skip_known=$((skip_known + 1))
    log_warning "Skipping known failure: $rel_path"
    continue
  fi

  file_ok=true
  file_spv=0

  for stage in $stages; do
    entry="${stage}Main"
    ext=$(get_output_ext "$stage")
    base=$(basename "$slang_file" .slang)
    outfile="$OUTDIR/${base}${ext}.spv"

    if ! "$SLANGC" "$slang_file" \
      -profile spirv_1_4 \
      -matrix-layout-column-major \
      -target spirv \
      -o "$outfile" \
      -entry "$entry" \
      -stage "$stage" \
      -warnings-disable 39001 2>"$OUTDIR/err.txt"; then
      file_ok=false
      err_msg=$(head -1 "$OUTDIR/err.txt")
      fail_details="${fail_details}  ${rel_path} (${stage}): ${err_msg}\n"
      break
    fi
    file_spv=$((file_spv + 1))
    rm -f "$outfile"
  done

  if [ "$file_ok" = true ]; then
    success=$((success + 1))
    spv_count=$((spv_count + file_spv))
  else
    fail=$((fail + 1))
  fi
done < <(find "$SHADER_DIR" -name "*.slang" -not -name "_*" | sort)

total=$((success + fail + skip + skip_known))

echo ""
echo "================================================"
echo "Sascha Willems Vulkan Shader Compilation Results"
echo "================================================"
echo ""
echo "Total .slang files:      $total"
echo "  Compiled OK:           $success"
echo "  Failed (unexpected):   $fail"
echo "  Skipped (known issue): $skip_known"
echo "  Skipped (no stage):    $skip"
echo "  SPIR-V generated:      $spv_count"
echo ""

if [ $fail -gt 0 ]; then
  echo "Unexpected failures:"
  echo -e "$fail_details"
fi

if [ $skip_known -gt 0 ]; then
  echo "Known failures (skipped):"
  for known in "${KNOWN_FAILURES[@]}"; do
    echo "  $known"
  done
  echo ""
fi

if [ $success -eq 0 ]; then
  if [ $total -eq 0 ]; then
    log_error "No .slang files found under $SHADER_DIR"
  else
    log_error "No shaders compiled successfully"
  fi
  exit 1
fi

if [ $fail -gt 0 ]; then
  log_error "$fail shader(s) failed unexpectedly"
  exit 1
else
  log_success "All $success shaders compiled successfully ($spv_count SPIR-V files)"
  if [ $skip_known -gt 0 ]; then
    log_warning "$skip_known shader(s) skipped due to known issues"
  fi
fi
