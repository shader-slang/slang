#!/usr/bin/env bash
set -e

show_help() {
  me=$(basename "$0")
  cat <<EOF
$me: Run all of the examples in test mode

Usage: $me --os <os> --config <config> --bin-dir <path> --skip-file <path> [--allow-file <path>] [--dry-run]

Options:
  --help                 Show this help message
  --dry-run              Skip running the examples.
  --bin-dir   <path>     Path to binary directory.
                         Must contain all of the example binaries.
  --skip-file <path>     Path to file containing skip patterns.
                         See the 'Skip file' section, below.
  --allow-file <path>    Optional path to a file listing example diagnostics
                         that are expected (and therefore tolerated). See the
                         'Diagnostics allow file' section, below. When omitted,
                         any compiler diagnostic printed by an example is
                         treated as a failure.
  --os        <os>       Operating system.
                         Valid <os> values: 'macos', 'windows', 'linux'
  --config    <config>   Build configuration.
                         Valid <config> values: 'debug', 'release'.

  --platform  <platform> Target platform.
                         Valid <platform> values: 'x86_64', 'aarch64'.

Skip file:

  The skip patterns are regexp patterns on the following format:
  <os>:<platform>:<config>:<sample> # Some comment describing why test is disabled

  For example:
  # Bug 123: foo-example fails (both debug and release)
  windows:x86_64(debug|release):foo-example
  # Bug 456: bar-example fails in release mode on Linux
  linux:aarch64:release:bar-example

Diagnostics allow file:

  An example is treated as failing if it prints a Slang compiler diagnostic
  (warning[...], error[...] or fatal error[...]), even when it still exits 0.
  The allow file lists diagnostics that are currently expected, so the runner
  does not flag them. Patterns use the same format as the skip file, with an
  optional trailing diagnostic code:
  <os>:<platform>:<config>:<sample>[:<code>] # Some comment describing why

  A pattern with no <code> allowlists every diagnostic from that sample.

  For example:
  # foo-example intentionally demonstrates a deprecation warning
  linux:x86_64:release:foo-example:E12345 # Intentional; see issue 999

EOF
}

function user_error() {
  echo "error: $1" >&2
  echo "" >&2
  show_help >&2
  exit 1
}

while [[ "$#" -gt 0 ]]; do
  case $1 in
  -h | --help)
    show_help
    exit 0
    ;;
  --dry-run)
    dry_run=true
    ;;
  --bin-dir)
    bin_dir="$2"
    shift
    ;;
  --skip-file)
    skip_file="$2"
    shift
    ;;
  --allow-file)
    allow_file="$2"
    shift
    ;;
  --os)
    case $2 in
    windows | linux | macos) ;;
    *)
      user_error "Unrecognized os: '$2'"
      ;;
    esac
    os="$2"
    shift
    ;;
  --config)
    case $2 in
    debug | release) ;;
    *)
      user_error "Unrecognized config: '$2'"
      ;;
    esac
    config="$2"
    shift
    ;;
  --platform)
    case $2 in
    x86_64 | aarch64) ;;
    *)
      user_error "Unrecognized platform: '$2'"
      ;;
    esac
    platform="$2"
    shift
    ;;
  *)
    user_error "Unrecognized argument: '$1'"
    ;;
  esac
  shift
done

if [[ "$os" == "" ]]; then
  user_error "No OS specified."
fi

if [[ "$config" == "" ]]; then
  user_error "No build configuration specified."
fi

if [[ "$bin_dir" == "" ]]; then
  user_error "No binary directory specified."
fi

if [[ "$skip_file" == "" ]]; then
  user_error "No skip file specified."
fi

if [[ "$platform" == "" ]]; then
  user_error "No platform specified."
fi

if [[ ! -f "$skip_file" ]]; then
  user_error "Skip file '$skip_file' does not exist."
fi

if [[ "$allow_file" != "" && ! -f "$allow_file" ]]; then
  user_error "Allow file '$allow_file' does not exist."
fi

if [[ ! -d "$bin_dir" ]]; then
  user_error "Binary directory '$bin_dir' does not exist."
fi

summary=()
failure_count=0
skip_count=0
sample_count=0

# Return success if $subject matches any comment-terminated regexp pattern in
# $pattern_file. Both the skip file and the diagnostics allow file share this
# format (`<pattern> # comment`), so a single matcher is the one source of
# truth for how patterns are parsed and applied.
function matches_pattern_file {
  local subject
  local pattern_file
  local line_index
  subject="$1"
  pattern_file="$2"
  line_index=1
  while read -r pattern; do
    # Ignore blank lines and standalone comment lines so files can be annotated
    # and grouped for readability.
    if [[ -z "${pattern// /}" || "${pattern#"${pattern%%[![:space:]]*}"}" == \#* ]]; then
      line_index=$((line_index + 1))
      continue
    fi
    pat=$pattern
    if [[ ! $pat =~ .*# ]]; then
      user_error "Pattern on line $line_index of '$pattern_file' is missing a comment!"
    fi
    pat="${pattern%% *#*}"
    if [[ $subject =~ ^$pat$ ]]; then
      return 0
    fi
    line_index=$((line_index + 1))
  done <"$pattern_file"

  return 1
}

function skip {
  matches_pattern_file "$1" "$skip_file"
}

# Regexp matching the Slang compiler-diagnostic line shape (e.g.
# `warning[E41017]:`, `error[E36108]:`, `fatal error[E40003]:`). Anchored to a
# line start or whitespace so it does not match the token appearing incidentally
# in an example's own text. check_sample_diagnostics isolates the bracketed code
# from each match afterwards.
diagnostic_regexp='(^|[[:space:]])(warning|error|fatal error)\[[A-Za-z]*[0-9]+\]:'

# Return success if the diagnostic $code from $sample is allowlisted, i.e. the
# allow file matches either `<os>:<platform>:<config>:<sample>` (all diagnostics
# from the sample allowed) or `<os>:<platform>:<config>:<sample>:<code>` (just
# this code). With no allow file, nothing is allowlisted.
function is_diagnostic_allowed {
  local sample
  local code
  sample="$1"
  code="$2"
  if [[ "$allow_file" == "" ]]; then
    return 1
  fi
  matches_pattern_file "$os:$platform:$config:$sample" "$allow_file" && return 0
  matches_pattern_file "$os:$platform:$config:$sample:$code" "$allow_file"
}

# Fail (return non-zero) if $output contains any Slang compiler diagnostic that
# is not allowlisted for $sample. Each offending diagnostic is echoed so the CI
# log explains exactly which example printed which code.
function check_sample_diagnostics {
  local sample
  local output
  local code
  local unexpected
  sample="$1"
  output="$2"
  unexpected=0
  while IFS= read -r code; do
    [[ -z "$code" ]] && continue
    if is_diagnostic_allowed "$sample" "$code"; then
      echo "note: '$sample' printed allowlisted diagnostic [$code]"
    else
      echo "error: '$sample' printed unexpected compiler diagnostic [$code]"
      unexpected=1
    fi
  done < <(echo "$output" | grep -oE "$diagnostic_regexp" | grep -oE '\[[A-Za-z]*[0-9]+\]:' | tr -d '[]:' | sort -u)
  return "$unexpected"
}

function run_sample {
  local sample
  local args
  sample="$1"
  shift
  args=("$@")
  sample_count=$((sample_count + 1))
  summary=("${summary[@]}" "$sample: ")
  if skip "$os:$platform:$config:$sample"; then
    echo "Skipping $sample..."
    summary=("${summary[@]}" "  skipped")
    skip_count=$((skip_count + 1))
    return
  fi
  echo "Running '$sample ${args[*]}'..."
  result=0
  local output
  output=""
  pushd "$bin_dir" 1>/dev/null 2>&1
  if [[ ! "$dry_run" = true ]]; then
    # Capture the sample's combined stdout+stderr while still echoing it to the
    # CI log via tee, so we can inspect it for compiler diagnostics below. Under
    # `set -e` a non-zero exit from the sample must not abort the script, hence
    # the `|| result=$?`; PIPESTATUS[0] recovers the sample's status past tee.
    output=$(
      ./"$sample" "${args[@]}" 2>&1 | tee /dev/stderr
      exit "${PIPESTATUS[0]}"
    ) || result=$?
    if [[ -f ./"log-$sample.txt" ]]; then
      output+=$'\n'$(cat ./"log-$sample.txt")
      cat ./"log-$sample.txt"
    fi
  fi
  popd 1>/dev/null 2>&1

  # An example must not print compiler diagnostics: even at exit 0 a printed
  # warning/error means the example is either buggy or demonstrating something
  # that should be fixed or explicitly allowlisted. Scan the captured output for
  # the Slang diagnostic shape (e.g. `warning[E41017]:`, `error[E36108]:`,
  # `fatal error[E40003]:`) and fail the sample on any diagnostic that is not
  # listed in the allow file.
  local diag_failure
  diag_failure=0
  if [[ "$result" -eq 0 && -n "$output" ]]; then
    check_sample_diagnostics "$sample" "$output" || diag_failure=$?
  fi

  if [[ $result -ne 0 ]]; then
    summary=("${summary[@]}" "  failure (exit code: $result)")
    failure_count=$((failure_count + 1))
  elif [[ $diag_failure -ne 0 ]]; then
    summary=("${summary[@]}" "  failure (unexpected compiler diagnostics)")
    failure_count=$((failure_count + 1))
  else
    summary=("${summary[@]}" "  success")
  fi
}

sample_commands=(
  'platform-test --test-mode'
  'ray-tracing-pipeline --test-mode'
  'ray-tracing --test-mode'
  'shader-toy --test-mode'
  'triangle --test-mode'
  'model-viewer --test-mode'
  'shader-object'
  'reflection-api'
  'hello-world'
  'gpu-printing'
  'cpu-hello-world'
  'cpu-com-example'
)

for sample_command in "${sample_commands[@]}"; do
  run_sample ${sample_command}
  echo ""
done

echo ""
echo "Summary: "
echo
for line in "${summary[@]}"; do
  printf '  %s\n' "$line"
done
echo ""
echo "$failure_count failed, and $skip_count skipped, out of $sample_count tests"
if [[ $failure_count -ne 0 ]]; then
  exit 1
fi
