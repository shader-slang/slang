#!/usr/bin/env bash

set -e

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(dirname "$script_dir")"

show_help() {
  me=$(basename "$0")
  cat <<EOF
$me: Build table of contents for documentation directories

Usage: $me [--help] [--source <path>]

Options:
    --help           Show this help message
    --source         Path to project root directory (defaults to parent of the script directory)
EOF
}

while [[ "$#" -gt 0 ]]; do
  case $1 in
  -h | --help)
    show_help
    exit 0
    ;;
  --source)
    project_root="$2"
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

missing_bin=0

require_bin() {
  local name="$1"
  if ! command -v "$name" &>/dev/null; then
    echo "This script needs $name, but it isn't in \$PATH" >&2
    missing_bin=1
    return
  fi
}

require_bin "mcs"
require_bin "mono"

if [ "$missing_bin" -eq 1 ]; then
  exit 1
fi

temp_dir=$(mktemp -d)
trap 'rm -rf "$temp_dir"' EXIT

cd "$project_root/docs" || exit 1

cat >"$temp_dir/temp_program.cs" <<EOL
$(cat "$script_dir/scripts/Program.cs")

namespace toc
{
    class Program
    {
        static int Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("Please provide a directory path");
                return 1;
            }

            try
            {
                Builder.Run(args[0]);
                return 0;
            }
            catch (Exception ex)
            {
                Console.WriteLine(\$"Error: {ex.Message}");
                return 1;
            }
        }
    }
}
EOL

if ! mcs -r:System.Core "$temp_dir/temp_program.cs" -out:"$temp_dir/toc-builder.exe"; then
  echo "Compilation failed"
  exit 1
fi

for dir in "user-guide" "gfx-user-guide"; do
  if [ -d "$script_dir/$dir" ]; then
    if ! mono "$temp_dir/toc-builder.exe" "$script_dir/$dir"; then
      echo "TOC generation failed for $dir"
      exit 1
    fi
  else
    echo "Directory $dir not found"
  fi
done
