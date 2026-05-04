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
    if [ -n "$githubIssue" ] || ! [[ $1 =~ ^[0-9]+$ ]]; then
      echo "Usage: $0 issue-number [--repo repo-name]" >&2
      exit 1
    fi
    githubIssue=$1
    ;;
  esac
  shift
done

if [ -z "$githubIssue" ]; then
  echo "Usage: $0 issue-number [--repo repo-name]" >&2
  exit 1
fi

log() { echo "[$(date '+%H:%M:%S')] $*" | tee -a "$mainLog"; }

for cmd in claude jq; do
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "Error: '$cmd' not found in PATH." >&2
    exit 1
  fi
done

# Use a per-issue subdirectory for plan files to avoid clobbering unrelated files in CWD
planDir="issue-$githubIssue"
mkdir -p "$planDir"
rm -fr "$planDir"/*
mainLog="$planDir/claude.log"

# Verify --dangerously-skip-permissions is effective before launching agents.
# Ask Claude to write a known value to a file; if the file is not created,
# the flag is blocked (e.g. by a managed policy) and we cannot proceed.
log "Checking --dangerously-skip-permissions..."
claude --dangerously-skip-permissions --print \
  "Run this bash command: printf 'ok' > $planDir/probe" >/dev/null 2>&1 || true
if [ ! -f "$planDir/probe" ] || [ "$(cat "$planDir/probe")" != "ok" ]; then
  log "Error: --dangerously-skip-permissions is not effective on this system."
  log "Check your Claude configuration or company policy. The script cannot continue."
  exit 1
fi
rm -f "$planDir/probe"
log "--dangerously-skip-permissions is working."

# Each angle drives one independent planning agent — add or remove entries to tune coverage
angles=(
  "Focus on root cause: trace the bug from symptom to source. Find similar past bugs in the same subsystem and how they were fixed."
  "Focus on test coverage: find similar issues resolved by adding tests. Identify what tests would catch this bug and prevent regressions. Check .github/workflows/ to ensure the new test is picked up by the appropriate CI job."
  "Focus on minimal change: find the smallest diff that fixes the issue without architectural side effects. Prefer targeted fixes over refactors."
  "Focus on architectural impact: find similar issues that required cross-file or cross-subsystem changes. Consider all call sites and dependents."
  "Focus on failure modes: find issues closed as wontfix or duplicates to understand what approaches were rejected and why."
  "Focus on the IR stage (slang-ir*.cpp/h): examine how the issue manifests in the IR, which IR pass introduces the problem, and which IR instructions or types are involved. Read extras/split-ir-dump.md for the IR dump debugging workflow and follow it to locate the culprit pass."
  "Focus on the emit stage (slang-emit*.cpp/h): examine whether the issue surfaces during code generation to HLSL/GLSL/SPIRV/Metal, and whether the fix belongs in an emitter."
  "Focus on the AST stage (slang-ast*.cpp/h, slang-check*.cpp): examine whether the issue is rooted in type-checking, name resolution, or semantic analysis before IR lowering."
  "Focus on the core-module (source/slang/*.meta.slang — core, hlsl, glsl, diff): examine whether the issue is rooted in built-in type definitions, intrinsics, or standard library functions defined in these modules."
  "Focus on the module/link stage (slang-ir-link.cpp, slang-serialize*.cpp, slang-module*.cpp): examine whether the issue surfaces during separate compilation, module serialization, or IR linking across translation units."
  "Focus on the legalization stage (slang-ir-legalize*.cpp, slang-ir-lower*.cpp): examine whether the issue is caused by a data representation transformation that Slang applies to meet target language requirements, such as struct splitting, array flattening, matrix layout conversion, or buffer element type lowering."
  "Focus on Slang language syntax (slang-parser.cpp, docs/user-guide/): examine whether the issue stems from incorrect user syntax, valid syntax that is not yet implemented, or a mismatch between what the spec allows and what the compiler accepts. Identify whether the fix belongs in the parser, the diagnostic messages, or the docs."
  "Focus on regression: examine git log and git blame to determine whether the issue was introduced by a recent change. Find the commit that broke the behavior, understand why it was made, and determine whether the fix should revert it, patch it, or take a different approach."
)
##############
# When you add more angles, please update claude.md document as well.
##############

agentCount=${#angles[@]}

log "Phase 1: Launching $agentCount planning agents in parallel..."

pids=()
for i in "${!angles[@]}"; do
  angle="${angles[$i]}"
  (
    claude --dangerously-skip-permissions --output-format json --print \
      "I want you to work on GitHub shader-slang/$githubRepo issue $githubIssue.
Research angle: $angle

1. Review the issue description, all comments, and every linked PR in detail.
2. Search for five similar issues and PRs on the same repo and study them thoroughly.
3. Apply your research angle to propose a concrete, actionable solution.
4. Write your full proposal to $planDir/plan.$i.md. Include:
   - Root cause analysis
   - Exact files and functions to change
   - Step-by-step implementation plan
   - Test cases to add or modify
   - Potential risks and how to mitigate them
5. Update documentation based on the nature of the information:
   - CLAUDE.md: generic information that helps LLMs work effectively in this repo — build commands, debugging workflows, tool usage, architectural navigation. No Slang language details here.
   - docs/user-guide/: user-facing information — how to use a language feature, what a construct means, usage examples, migration guidance.
   - docs/language-reference/: technical or developer-facing details — compiler behavior, formal semantics, design decisions, internal architecture." \
      >"$planDir/claude.$i.json" 2>&1 || true
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

plan_count=$(find "$planDir" -maxdepth 1 -name 'plan.[0-9]*.md' | wc -l)
log "Phase 1 complete: $plan_count / $agentCount plans produced ($failed failed)"

if [ "$plan_count" -eq 0 ]; then
  log "Error: No plan files produced. Exiting."
  exit 1
fi

log "Phase 2: Synthesizing best solution from $plan_count plans..."

claude --dangerously-skip-permissions --output-format json --print \
  "I want you to work on GitHub shader-slang/$githubRepo issue $githubIssue.

Phase: Solution Synthesis

1. Read every $planDir/plan.*.md file in the current directory. Each was written by an independent agent with a different research angle.
2. Compare proposals: identify where they agree (high-confidence areas), where they diverge (areas requiring judgment), and which plan has the strongest root cause analysis.
3. Produce $planDir/plan.best.md — a synthesized plan that takes the strongest elements from each. It must include:
   - Final root cause analysis
   - Ordered list of files and functions to change
   - Concrete implementation steps
   - Test cases to add or modify
   - Known risks and mitigations
4. Do NOT write any code yet." \
  >"$planDir/claude.best.json"
jq -r '.result // empty' "$planDir/claude.best.json" | tee -a "$mainLog"

if [ ! -f "$planDir/plan.best.md" ]; then
  log "Warning: plan.best.md not produced. Falling back to first available plan."
  first_plan=$(find "$planDir" -maxdepth 1 -name 'plan.[0-9]*.md' | sort | head -1)
  if [ -z "$first_plan" ]; then
    log "Error: no plan available."
    exit 1
  fi
  cp "$first_plan" "$planDir/plan.best.md"
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
  local tmp="$planDir/claude.impl.json"
  claude --dangerously-skip-permissions --output-format json "${args[@]}" --print "$prompt" >"$tmp"
  jq -r '.result // empty' "$tmp" | tee -a "$mainLog" >&2
  jq -r '.session_id // empty' "$tmp"
}

log "Phase 3: Implementing and committing..."

impl_session=$(claude_json -- \
  "I want you to work on GitHub shader-slang/$githubRepo issue $githubIssue.

Phase: Implementation

1. Read $planDir/plan.best.md carefully — this is your implementation blueprint.
2. Re-read the issue and any linked PRs to confirm your understanding before touching code.
3. Implement the solution exactly as specified in $planDir/plan.best.md.
4. Write $planDir/review-guide.md to help reviewers understand the PR before reading the diff. Include:
   - What the problem is (in detail)
   - Root cause of the problem
   - The approach taken and why
   - Implementation details
   - Alternative solutions considered and why they were rejected
   Use examples, ASCII diagrams, and any other visualization that helps reviewers understand quickly.
5. Stage all modified files with git add and commit. The commit message must be brief and have:
   - a short and end-user-friendly description of the problem,
   - a short and end-user-friendly description of the root-cause/analysis,
   - a short description without jargon of the approach and ideas to the solution,
   - (optional) technical details for reviewers.")

if [ -z "$impl_session" ]; then
  log "Error: Implementation phase did not return a session ID. Exiting."
  exit 1
fi

log "Implementation session ID: $impl_session"

log "Phase 4: Virtual review and amending..."

claude --dangerously-skip-permissions --output-format json --print \
  "I want you to do a virtual code review of the implementation for GitHub shader-slang/$githubRepo issue $githubIssue.

1. Review git show to see the committed changeset.
2. Walk through the issue scenario step by step and confirm the implementation resolves it.
3. Identify all concerns a real code reviewer would raise: correctness, style, edge cases, test coverage, and potential regressions.
4. Review all documentation changes and revert any additions that are not significant enough to justify a permanent change or are unrelated to the final implementation.
5. Review new source code comments: ensure they match the implementation and are concise.
6. Fix every concern you identified.
7. Run git commit --amend to incorporate all fixes into the original commit." \
  >"$planDir/claude.review.json"
jq -r '.result // empty' "$planDir/claude.review.json" | tee -a "$mainLog"

log "Done."

# Print total token and cost statistics across all phases
raw_files=()
for f in "$planDir"/claude.*.json; do
  [ -f "$f" ] && raw_files+=("$f")
done
if [ ${#raw_files[@]} -gt 0 ] && command -v jq >/dev/null 2>&1; then
  stats=$(jq -s '
    {
      input:        ([.[].usage.input_tokens              // 0] | add // 0),
      output:       ([.[].usage.output_tokens             // 0] | add // 0),
      cache_read:   ([.[].usage.cache_read_input_tokens   // 0] | add // 0),
      cache_write:  ([.[].usage.cache_creation_input_tokens // 0] | add // 0),
      cost:         ([.[].total_cost_usd // .[].cost_usd  // 0] | add // 0)
    }
  ' "${raw_files[@]}" 2>/dev/null) || stats=""
  if [ -n "$stats" ]; then
    log "Token/cost statistics (all phases):"
    log "  Input tokens:        $(echo "$stats" | jq '.input')"
    log "  Output tokens:       $(echo "$stats" | jq '.output')"
    log "  Cache read tokens:   $(echo "$stats" | jq '.cache_read')"
    log "  Cache write tokens:  $(echo "$stats" | jq '.cache_write')"
    log "  Total cost (USD):   \$$(echo "$stats" | jq '.cost')"
  fi
fi
log ""
log "To resume the implementation session interactively:
  claude --dangerously-skip-permissions --resume $impl_session"
