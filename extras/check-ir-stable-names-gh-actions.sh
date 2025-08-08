#!/usr/bin/env bash
set -e

# The github action runners don't have libreadline and we don't need it
make -C external/lua MYCFLAGS="-DLUA_USE_POSIX" MYLIBS=""
external/lua/lua -v

# Run the check
if ! ./external/lua/lua extras/check-ir-stable-names.lua check; then
  echo "Check failed. Running update..."
  ./external/lua/lua extras/check-ir-stable-names.lua update

  echo -e "\n=== Diff of changes made ==="
  git diff --color=always source/slang/slang-ir-insts-stable-names.lua || true

  # Also create a summary for GitHub Actions
  if [ -n "$GITHUB_STEP_SUMMARY" ]; then
    {
      echo "## IR Stable Names Table Update Required"
      echo "The following changes need to be made to \`source/slang/slang-ir-insts-stable-names.lua\`:"
      echo '```diff'
      git diff source/slang/slang-ir-insts-stable-names.lua
      echo '```'
    } >>"$GITHUB_STEP_SUMMARY"
  fi

  # Fail the job since the check failed
  exit 1
fi
