#!/usr/bin/env bash

set -e

# Check Bash version
if [ "${BASH_VERSINFO[0]}" -lt 4 ]; then
  echo "Error: Bash 4 or newer is required. Current version: $BASH_VERSION" >&2
  if [[ "$(uname)" == "Darwin" ]]; then
    echo "Please install a newer version of Bash using Homebrew:" >&2
    echo "  brew install bash" >&2
  fi
  exit 1
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
source_dir="$(dirname "$script_dir")"
since_rev=""

# Detect macOS and set appropriate binary names
if [[ "$(uname)" == "Darwin" ]]; then
  # On macOS, check for GNU versions
  # grep and xargs use g-prefix, diff is installed as /opt/homebrew/bin/diff
  missing_tools=()

  if ! command -v ggrep &>/dev/null; then
    missing_tools+=("grep")
  fi

  if ! command -v gxargs &>/dev/null; then
    missing_tools+=("findutils")
  fi

  if ! command -v /opt/homebrew/bin/diff &>/dev/null; then
    missing_tools+=("diffutils")
  fi

  if [ ${#missing_tools[@]} -gt 0 ]; then
    echo "Error: GNU versions of grep, xargs, and diff are required on macOS." >&2
    echo "Please install them using Homebrew:" >&2
    echo "  brew install ${missing_tools[*]}" >&2
    exit 1
  fi

  GREP_BIN="ggrep"
  XARGS_BIN="gxargs"
  DIFF_BIN="/opt/homebrew/bin/diff"
else
  # On other systems, use standard binaries
  GREP_BIN="grep"
  XARGS_BIN="xargs"
  DIFF_BIN="diff"
fi

check_only=0
no_version_check=0
modified_files=0
run_cpp=0
run_yaml=0
run_markdown=0
run_sh=0
run_cmake=0
run_all=1

show_help() {
  me=$(basename "$0")
  cat <<EOF
$me: Format or check formatting of files in this repo

Usage: $me [--check-only] [--no-version-check] [--source <path>] [--cpp] [--yaml] [--md] [--sh] [--cmake]

Options:
    --check-only       Check formatting without modifying files
    --no-version-check Skip version compatibility checks
    --source          Path to source directory to format (defaults to parent of script directory)
    --cpp             Format only C++ files
    --yaml            Format only YAML/JSON files
    --md              Format only markdown files
    --sh              Format only shell script files
    --cmake           Format only CMake files
    --since <rev>     Only format files since Git revision <rev>
EOF
}

while [[ "$#" -gt 0 ]]; do
  case $1 in
  -h | --help)
    show_help
    exit 0
    ;;
  --check-only) check_only=1 ;;
  --no-version-check) no_version_check=1 ;;
  --modified) modified_files=1 ;;
  --cpp)
    run_cpp=1
    run_all=0
    ;;
  --yaml)
    run_yaml=1
    run_all=0
    ;;
  --md)
    run_markdown=1
    run_all=0
    ;;
  --sh)
    run_sh=1
    run_all=0
    ;;
  --cmake)
    run_cmake=1
    run_all=0
    ;;
  --source)
    source_dir="$2"
    shift
    ;;
  --since)
    since_rev="$2"
    shift
    ;;
  *)
    echo "unrecognized argument: $1"
    show_help
    exit 1
    ;;
  esac
  shift
done

cd "$source_dir" || exit 1

require_bin() {
  local name="$1"
  local min_version="$2"
  local max_version="${3:-}"
  local version

  if ! command -v "$name" &>/dev/null; then
    echo "This script needs $name, but it isn't in \$PATH" >&2
    missing_bin=1
    return
  fi

  if [ "$no_version_check" -eq 0 ]; then
    version=$("$name" --version | $GREP_BIN -oP "\d+\.\d+\.?\d*" | head -n1)

    # Debug output to stderr
    if [ -n "$max_version" ]; then
      echo "found $name $version, required [$min_version, $max_version)" >&2
    else
      echo "found $name $version, required at least $min_version" >&2
    fi

    if ! printf '%s\n%s\n' "$min_version" "$version" | sort -V -C; then
      echo "$name version $version is too old. Version $min_version or newer is required." >&2
      missing_bin=1
      return
    fi

    if [ -n "$max_version" ]; then
      if ! printf '%s\n%s\n' "$version" "$max_version" | sort -V -C; then
        echo "$name version $version is too new. Version less than $max_version is required." >&2
        missing_bin=1
        return
      fi
    fi
  fi
}

require_bin "git" "1.8"
((run_all || run_cmake)) && require_bin "gersemi" "0.21" "0.22"
((run_all || run_cpp)) && require_bin "$XARGS_BIN" "3"
require_bin "$DIFF_BIN" "2"
((run_all || run_cpp)) && require_bin "clang-format" "17" "18"
((run_all || run_yaml || run_markdown)) && require_bin "prettier" "3"
((run_all || run_sh)) && require_bin "shfmt" "3"

if [ "$missing_bin" ]; then
  exit 1
fi

get_nproc() {
  local nproc_count
  if command -v nproc &>/dev/null; then
    nproc_count=$(nproc)
  elif [[ "$(uname)" == "Darwin" ]] && command -v sysctl &>/dev/null; then
    nproc_count=$(sysctl -n hw.logicalcpu)
  elif command -v getconf &>/dev/null; then
    nproc_count=$(getconf _NPROCESSORS_ONLN 2>/dev/null)
  fi
  echo "${nproc_count:-1}"
}

exit_code=0

function list_files() {
  if [ "$since_rev" ] || [ "$modified_files" -eq 1 ]; then
    command="git diff --name-only"
    if [ "$since_rev" ]; then
      command="$command $since_rev"
    else
      command="$command HEAD"
    fi
    if [ "$modified_files" -eq 0 ]; then
      command="$command HEAD"
    fi
    $command "$@"
  else
    git ls-files $@
  fi
}

cmake_formatting() {
  echo "Formatting CMake files..." >&2

  readarray -t files < <(list_files '*.cmake' 'CMakeLists.txt' '**/CMakeLists.txt')

  common_args=(
    # turn on warning when this is fixed https://github.com/BlankSpruce/gersemi/issues/39
    --no-warn-about-unknown-commands
    --definitions "${files[@]}"
  )

  if [ "$check_only" -eq 1 ]; then
    gersemi "${common_args[@]}" --diff --color "${files[@]}"
    gersemi "${common_args[@]}" --check "${files[@]}" || exit_code=1
  else
    gersemi "${common_args[@]}" --in-place "${files[@]}"
  fi
}

track_progress() {
  # Don't output the progress bar if stderr isn't a terminal, just eat all the input
  [ -t 2 ] || {
    cat >/dev/null
    return
  }

  local total=$1
  local current=0

  ((total)) && while IFS= read -r _; do
    ((current++)) || :
    percent=$((current * 100 / total))
    printf '\rProgress: [%-50s] %d%%' "$(printf '#%.0s' $(seq 1 $((percent / 2))))" "$percent" >&2
  done
  echo >&2
}

cpp_formatting() {
  echo "Formatting cpp files..." >&2

  readarray -t files < <(list_files '*.cpp' '*.hpp' '*.c' '*.h' ':!external/**')

  # The progress reporting is a bit sneaky, we use `--verbose` with xargs which
  # prints a line to stderr for each command, and we simply count these...

  if [ "$check_only" -eq 1 ]; then
    local tmpdir
    tmpdir=$(mktemp -d)
    trap 'rm -rf "$tmpdir"' EXIT

    printf '%s\n' "${files[@]}" | $XARGS_BIN --verbose -P "$(get_nproc)" -I{} bash -c "
      mkdir -p \"\$(dirname \"$tmpdir/{}\")\"
      $DIFF_BIN -u --color=always --label \"{}\" --label \"{}\" \"{}\" <(clang-format \"{}\") > \"$tmpdir/{}\"
      :
    " |& track_progress ${#files[@]}

    for file in "${files[@]}"; do
      # Fail if any of the diffs have contents
      if [ -s "$tmpdir/$file" ]; then
        cat "$tmpdir/$file"
        exit_code=1
      fi
    done
  else
    printf '%s\n' "${files[@]}" | $XARGS_BIN --verbose -n1 -P "$(get_nproc)" clang-format -i |&
      track_progress ${#files[@]}
  fi
}

# Format the 'files' array using the prettier tool (abstracted here because
# it's used by markdown and json
prettier_formatting() {
  if [ "$check_only" -eq 1 ]; then
    for file in "${files[@]}"; do
      if ! output=$(prettier "$file" 2>/dev/null); then
        continue
      fi
      if ! $DIFF_BIN -q "$file" <(echo "$output") >/dev/null 2>&1; then
        $DIFF_BIN --color -u --label "$file" --label "$file" "$file" <(echo "$output") || :
        exit_code=1
      fi
    done
  else
    prettier --write "${files[@]}" | $GREP_BIN -v '(unchanged)' >&2 || :
  fi
}

yaml_json_formatting() {
  echo "Formatting yaml and json files..." >&2

  readarray -t files < <(list_files "*.yaml" "*.yml" "*.json" ':!external/**')

  prettier_formatting
}

markdown_formatting() {
  echo "Formatting markdown files..." >&2

  readarray -t files < <(list_files "*.md" ':!external/**')

  prettier_formatting
}

sh_formatting() {
  echo "Formatting sh files..." >&2

  readarray -t files < <(list_files "*.sh")

  common_args=(
    # default 8 is way too wide
    --indent 2
  )

  if [ "$check_only" -eq 1 ]; then
    shfmt "${common_args[@]}" --diff "${files[@]}" || exit_code=1
  else
    shfmt "${common_args[@]}" --write "${files[@]}"
  fi
}

((run_all || run_sh)) && sh_formatting
((run_all || run_cmake)) && cmake_formatting
((run_all || run_yaml)) && yaml_json_formatting
((run_markdown)) && markdown_formatting
((run_all || run_cpp)) && cpp_formatting

exit $exit_code
