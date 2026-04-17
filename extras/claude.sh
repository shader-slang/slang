#!/bin/bash
set -euo pipefail

githubRepo="slang"
githubIssue=""

while [[ $# -gt 0 ]]; do
  case $1 in
  --repo)
    if [[ $# -lt 2 || -z "${2:-}" ]]; then
      echo "Usage: $0 issue-number [--repo repo-name]" >&2
      exit 1
    fi
    githubRepo=$2
    shift
    ;;
  -*)
    echo "Usage: $0 issue-number [--repo repo-name]" >&2
    exit 1
    ;;
  *)
    githubIssue=$1
    ;;
  esac
  shift
done

if [ -z "$githubIssue" ]; then
  echo "Usage: $0 issue-number [--repo repo-name]" >&2
  exit 1
fi

log() { echo "[$(date '+%H:%M:%S')] $*"; }

# Clean up plan files from any previous run in this directory
rm -f plan.*.md plan.best.md

# Each angle drives one independent planning agent — add or remove entries to tune coverage
angles=(
  "Focus on root cause: trace the bug from symptom to source. Find similar past bugs in the same subsystem and how they were fixed."
  "Focus on test coverage: find similar issues resolved by adding tests. Identify what tests would catch this bug and prevent regressions."
  "Focus on minimal change: find the smallest diff that fixes the issue without architectural side effects. Prefer targeted fixes over refactors."
  "Focus on architectural impact: find similar issues that required cross-file or cross-subsystem changes. Consider all call sites and dependents."
  "Focus on failure modes: find issues closed as wontfix or duplicates to understand what approaches were rejected and why."
  "Focus on the IR stage (slang-ir*.cpp/h): examine how the issue manifests in the IR, which IR pass introduces the problem, and which IR instructions or types are involved. Read extras/split-ir-dump.md for the IR dump debugging workflow and follow it to locate the culprit pass."
  "Focus on the emit stage (emit*.cpp/h): examine whether the issue surfaces during code generation to HLSL/GLSL/SPIRV/Metal, and whether the fix belongs in an emitter."
  "Focus on the AST stage (slang-ast*.cpp/h, check*.cpp): examine whether the issue is rooted in type-checking, name resolution, or semantic analysis before IR lowering."
  "Focus on the core-module (source/slang/*.meta.slang — core, hlsl, glsl, diff): examine whether the issue is rooted in built-in type definitions, intrinsics, or standard library functions defined in these modules."
  "Focus on the module/link stage (slang-ir-link.cpp, slang-serialize*.cpp, slang-module*.cpp): examine whether the issue surfaces during separate compilation, module serialization, or IR linking across translation units."
  "Focus on the legalization stage (slang-ir-legalize*.cpp, slang-ir-lower*.cpp): examine whether the issue is caused by a data representation transformation that Slang applies to meet target language requirements, such as struct splitting, array flattening, matrix layout conversion, or buffer element type lowering."
  "Focus on Slang language syntax (slang-parser.cpp, docs/user-guide/, external/spec/): examine whether the issue stems from incorrect user syntax, valid syntax that is not yet implemented, or a mismatch between what the spec allows and what the compiler accepts. Identify whether the fix belongs in the parser, the diagnostic messages, or the spec/docs."
)

agentCount=${#angles[@]}

log "Phase 1: Launching $agentCount planning agents in parallel..."

pids=()
for i in "${!angles[@]}"; do
  angle="${angles[$i]}"
  (
    claude --dangerously-skip-permissions --print \
      "I want you to work on GitHub shader-slang/$githubRepo issue $githubIssue.
Research angle: $angle

1. Review the issue description, all comments, and every linked PR in detail.
2. Search for five similar issues and PRs on the same repo and study them thoroughly.
3. Apply your research angle to propose a concrete, actionable solution.
4. Write your full proposal to plan.$i.md. Include:
   - Root cause analysis
   - Exact files and functions to change
   - Step-by-step implementation plan
   - Test cases to add or modify
   - Potential risks and how to mitigate them
5. If CLAUDE.md is missing information that would meaningfully improve LLM effectiveness on this repo, add it."
  ) &
  pids+=($!)
  log "  Agent $i launched (PID ${pids[-1]}): ${angle:0:70}..."
done

log "Waiting for all planning agents..."
failed=0
for i in "${!pids[@]}"; do
  if ! wait "${pids[$i]}" 2>/dev/null; then
    log "  Warning: agent $((i + 1)) failed (PID ${pids[$i]})"
    ((failed++)) || true
  fi
done

plan_count=$(find . -maxdepth 1 -name 'plan.[0-9]*.md' | wc -l)
log "Phase 1 complete: $plan_count / $agentCount plans produced ($failed failed)"

if [ "$plan_count" -eq 0 ]; then
  log "Error: No plan files produced. Exiting."
  exit 1
fi

log "Phase 2: Synthesizing best solution from $plan_count plans..."

claude --dangerously-skip-permissions --print \
  "I want you to work on GitHub shader-slang/$githubRepo issue $githubIssue.

Phase: Solution Synthesis

1. Read every plan.*.md file in the current directory. Each was written by an independent agent with a different research angle.
2. Compare proposals: identify where they agree (high-confidence areas), where they diverge (areas requiring judgment), and which plan has the strongest root cause analysis.
3. Produce plan.best.md — a synthesized plan that takes the strongest elements from each. It must include:
   - Final root cause analysis
   - Ordered list of files and functions to change
   - Concrete implementation steps
   - Test cases to add or modify
   - Known risks and mitigations
4. Do NOT write any code yet."

if [ ! -f plan.best.md ]; then
  log "Warning: plan.best.md not produced. Falling back to first available plan."
  first_plan=$(find . -maxdepth 1 -name 'plan.[0-9]*.md' | sort | head -1)
  if [ -z "$first_plan" ]; then
    log "Error: no plan available."
    exit 1
  fi
  cp "$first_plan" plan.best.md
fi

# Runs claude with --output-format json and emits:
#   stdout: the session_id (sole value, safe to capture with $())
#   stderr: the result text (visible to the user)
# Usage: session=$(claude_json [flags...] -- "prompt")
claude_json() {
  local args=() prompt="" seen_sep=0
  while (($#)); do
    if [ "$1" = "--" ]; then
      seen_sep=1
      shift
      prompt="$1"
      shift
      continue
    fi
    args+=("$1")
    shift
  done
  ((seen_sep)) || {
    echo "claude_json: missing -- before prompt" >&2
    return 2
  }
  local tmp
  tmp=$(mktemp -t claude.XXXXXX.json) || return 1
  claude --dangerously-skip-permissions --output-format json "${args[@]}" --print "$prompt" >"$tmp"
  jq -r '.result // empty' "$tmp" >&2
  jq -r '.session_id // empty' "$tmp"
  rm -f "$tmp"
}

log "Phase 3: Implementing the solution..."

impl_session=$(claude_json -- \
  "I want you to work on GitHub shader-slang/$githubRepo issue $githubIssue.

Phase: Implementation

1. Read plan.best.md carefully — this is your implementation blueprint.
2. Re-read the issue and any linked PRs to confirm your understanding before touching code.
3. Implement the solution exactly as specified in plan.best.md.
4. Do NOT commit changes yet." | tail -1)

if [ -z "$impl_session" ]; then
  log "Error: Implementation phase did not return a session ID. Exiting."
  exit 1
fi

log "Phase 4: Refining the implementation..."

claude_json --resume "$impl_session" -- \
  "Continue working on GitHub shader-slang/$githubRepo issue $githubIssue.

Phase: Refinement

1. Run git diff to review all changes made so far.
2. Check for: unhandled edge cases, missing error handling at boundaries, style inconsistencies with surrounding code, and any regression risks in modified code paths.
3. Apply all improvements directly — do not just describe them." >/dev/null

log "Phase 5: Virtual review..."

claude --dangerously-skip-permissions --print \
  "I want you to do a virtual code review of the implementation for GitHub shader-slang/$githubRepo issue $githubIssue.

1. Review git diff to see the complete changeset.
2. Walk through the issue scenario step by step and confirm the implementation resolves it.
3. Identify all concerns a real code reviewer would raise: correctness, style, edge cases, test coverage, and potential regressions.
4. Review any changes to CLAUDE.md and revert additions that are not significant enough to justify a permanent change.
5. Fix every concern you identified directly in the code. Do NOT commit."

log "Phase 6: Committing the changes..."

claude --dangerously-skip-permissions --resume "$impl_session" --print \
  "Commit all changes for GitHub shader-slang/$githubRepo issue $githubIssue.

1. Run git diff to review the full changeset one final time.
2. Stage all modified files with git add.
3. Commit with a brief, descriptive message that summarises what was fixed and why."

log "Done."
echo ""
echo "To resume the implementation session interactively:"
echo "  claude --dangerously-skip-permissions --resume $impl_session"
