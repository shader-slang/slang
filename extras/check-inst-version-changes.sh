#!/usr/bin/env bash

set -euo pipefail

# Enable debug mode if DEBUG env var is set
DEBUG="${DEBUG:-false}"

debug_log() {
  if [[ "$DEBUG" == "true" ]]; then
    echo "[DEBUG] $*" >&2
  fi
}

# Check if running in GitHub Actions
if [[ -z "${GITHUB_ACTIONS:-}" ]]; then
  echo "This script is designed to run in GitHub Actions"
  exit 0
fi

# Check for required tools
if ! command -v gh &>/dev/null; then
  echo "Error: GitHub CLI (gh) is not installed"
  exit 1
fi

# Verify GitHub authentication
if ! gh auth status &>/dev/null; then
  echo "Error: Not authenticated with GitHub CLI"
  exit 1
fi

# Function to check if module ir version constants were modified
check_module_versions_modified() {
  local base_ref="$1"

  # Check if slang-ir.h was modified and if the version constants were changed
  if git diff --name-only "$base_ref...HEAD" | grep -q "^source/slang/slang-ir\.h$"; then
    # Check if either version constant was modified
    if git diff "$base_ref...HEAD" -- source/slang/slang-ir.h | grep -E "^\+.*k_(min|max)SupportedModuleVersion\s*="; then
      debug_log "Module version constants were modified"
      return 0
    fi
  fi

  debug_log "Module version constants were not modified"
  return 1
}

# Function to check if serialization version was modified
check_serialization_version_modified() {
  local base_ref="$1"

  # Check if the serialization version constant was modified
  if git diff "$base_ref...HEAD" -- source/slang/slang-serialize-ir.cpp | grep -E "^\+.*kSupportedSerializationVersion\s*="; then
    debug_log "Serialization version constant was modified"
    return 0
  fi

  debug_log "Serialization version constant was not modified"
  return 1
}

# Get PR number if this is a pull request
PR_NUMBER=""
if [[ "$GITHUB_EVENT_NAME" == "pull_request" ]]; then
  PR_NUMBER="${GITHUB_EVENT_PULL_REQUEST_NUMBER:-${GITHUB_PULL_REQUEST_NUMBER:-}}"
  if [[ -z "$PR_NUMBER" ]] && [[ -f "${GITHUB_EVENT_PATH:-}" ]]; then
    PR_NUMBER=$(jq -r '.pull_request.number // empty' "$GITHUB_EVENT_PATH" 2>/dev/null || echo "")
  fi
fi

debug_log "Event name: $GITHUB_EVENT_NAME"
debug_log "PR number: ${PR_NUMBER:-<none>}"

# Get the base ref for comparison
if [[ "$GITHUB_EVENT_NAME" == "pull_request" ]]; then
  BASE_REF="origin/${GITHUB_BASE_REF}"
  debug_log "Fetching base ref: $GITHUB_BASE_REF"
  if ! git fetch origin "$GITHUB_BASE_REF" --depth=1; then
    echo "Warning: Failed to fetch base ref, trying without depth limit"
    git fetch origin "$GITHUB_BASE_REF"
  fi
else
  BASE_REF="HEAD^1"
fi

debug_log "Base ref for comparison: $BASE_REF"

# Get list of changed files
CHANGED_FILES=$(git diff --name-only "$BASE_REF...HEAD" || echo "")
debug_log "Changed files:"
debug_log "$CHANGED_FILES"

# Check for changes in IR instruction files
INST_FILES_CHANGED=false
if echo "$CHANGED_FILES" | grep -E "^source/slang/slang-ir-insts(-stable-names)?\.lua$"; then
  INST_FILES_CHANGED=true
  debug_log "IR instruction files have changed"
fi

# Check for changes in serialization file
SERIALIZE_CHANGED=false
if echo "$CHANGED_FILES" | grep -q "^source/slang/slang-serialize-ir\.cpp$"; then
  SERIALIZE_CHANGED=true
  debug_log "Serialization file has changed"
fi

# Initialize comment body
COMMENT_BODY=""
NEEDS_COMMENT=false

# Check if we need to add warnings
if [[ "$INST_FILES_CHANGED" == "true" ]]; then
  # Check if the version constants have already been updated
  if check_module_versions_modified "$BASE_REF"; then
    echo "::notice::IR instruction files changed but module version constants were already updated"
  else
    NEEDS_COMMENT=true
    if [[ -n "$COMMENT_BODY" ]]; then
      COMMENT_BODY="${COMMENT_BODY}

"
    fi
    COMMENT_BODY="${COMMENT_BODY}⚠️ **IR Instruction Files Changed**

This PR modifies IR instruction definition files. Please review if you need to update the following constants in \`source/slang/slang-ir.h\`:

- \`k_minSupportedModuleVersion\`: Should be incremented if you're removing instructions or making breaking changes
- \`k_maxSupportedModuleVersion\`: Should be incremented when adding new instructions

These version numbers help ensure compatibility between different versions of compiled modules."

    echo "::warning::IR instruction files changed - please check if module version constants need updating"
  fi
fi

if [[ "$SERIALIZE_CHANGED" == "true" ]]; then
  # Check if the serialization version has already been updated
  if check_serialization_version_modified "$BASE_REF"; then
    echo "::notice::Serialization code changed but serialization version was already updated"
  else
    NEEDS_COMMENT=true
    if [[ -n "$COMMENT_BODY" ]]; then
      COMMENT_BODY="${COMMENT_BODY}

"
    fi
    COMMENT_BODY="${COMMENT_BODY}⚠️ **Serialization Code Changed**

This PR modifies \`source/slang/slang-serialize-ir.cpp\`. Please review if you need to update:

- \`kSupportedSerializationVersion\`: Should be incremented if you're making backwards-incompatible changes to the serialization format

This version number helps maintain compatibility when loading serialized IR modules."

    echo "::warning::Serialization code changed - please check if serialization version needs updating"
  fi
fi

# Create artifact directory only if we need to comment
if [[ "$NEEDS_COMMENT" == "true" ]] && [[ -n "$PR_NUMBER" ]]; then
  ARTIFACT_DIR="${ARTIFACT_DIR:-ir-version-check-artifact}"
  debug_log "Creating artifact directory: $ARTIFACT_DIR"
  mkdir -p "$ARTIFACT_DIR"

  # Write artifact files
  echo "$PR_NUMBER" >"$ARTIFACT_DIR/pr-number.txt"

  # Write comment body with marker
  {
    echo "<!-- slang-ir-version-check -->"
    echo "$COMMENT_BODY"
  } >"$ARTIFACT_DIR/comment-body.txt"

  debug_log "Artifact files created:"
  debug_log "  pr-number: $PR_NUMBER"
  debug_log "  comment-body: $(wc -l <"$ARTIFACT_DIR/comment-body.txt") lines"

  # Set output to indicate artifact was created
  if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
    echo "artifact_created=true" >>"$GITHUB_OUTPUT"
  else
    echo "::set-output name=artifact_created::true"
  fi
else
  debug_log "No artifact needed (needs_comment=$NEEDS_COMMENT, pr_number=${PR_NUMBER:-<empty>})"

  # Set output to indicate no artifact was created
  if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
    echo "artifact_created=false" >>"$GITHUB_OUTPUT"
  else
    echo "::set-output name=artifact_created::false"
  fi
fi

debug_log "Script completed successfully"
exit 0
