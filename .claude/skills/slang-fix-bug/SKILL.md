---
name: slang-fix-bug
description: Analyze a Slang compiler bug from a test failure, CI log, or GitHub issue, investigate root causes in depth, explore alternative fixes in parallel, present options to the user, and implement the chosen fix with tests and a PR.
---

# Slang Bug Fix

**For**: Diagnosing and fixing Slang compiler bugs -- from symptom to root cause to validated fix.

**Core Principle**: Dig into the root cause. Never apply a band-aid fix without understanding
why the bug exists and what invariants are violated. Present alternative fix strategies to the
user before implementing.

**Usage**: `/slang-fix-bug <bug-source> [--parallel N]`

Where `<bug-source>` is one of:
- A GitHub issue number or URL (e.g., `#10419` or `https://github.com/shader-slang/slang/issues/10419`)
- A test file path (e.g., `tests/bugs/my-failing-test.slang`)
- A CI log URL or terminal output
- A description of the symptom

Options:
- `--parallel N`: Launch N parallel fix agents (default: sequential, one at a time)

---

## Phase 1: INTAKE -- Understand the Symptom

Gather all available information about the bug from the provided source.

### From a GitHub Issue

```bash
gh issue view <number> --repo shader-slang/slang --json title,body,labels,comments
```

Extract: reproducer code (mandatory), target(s) affected, error message, expected vs actual behavior.

### From a CI Log / Local Test / Description

Parse or ask for: minimal reproducer, target(s), error output.

### Intake Output

Write `tmp/<issue-repository>-<bug-id>/intake.md` so the investigation trail
is preserved for rewinding or handoff.

```markdown
# Bug Intake: [short title]

## Source
[GitHub issue #N / CI log / local test / user description]

## Reproducer
[minimal code + command]

## Error Output
[exact error message, crash output, or wrong behavior]

## Expected Behavior
[what should happen]

## Affected Targets
[spirv, hlsl, cuda, cpu, metal, wgsl, all]

## Initial Classification
- [ ] ICE (Internal Compiler Error / error 99999)
- [ ] Wrong codegen
- [ ] Missing diagnostic
- [ ] SPIRV validation error
- [ ] Crash / segfault / assertion failure
```

---

## Phase 2: ROOT CAUSE INVESTIGATION

Run the `slang-investigate` skill on the bug source. This produces `tmp/<bug-id>/investigation.md`
with crash site, code path, violated invariant, design context, and potential fix locations.

If a GitHub issue exists, post a summary of the investigation as a comment (prefix with
`[Agent]`). This shares the analysis with other contributors.

See the `slang-investigate` skill for the full investigation methodology.

---

## Phase 3: EXPLORE ALTERNATIVES

Decide whether to explore sequentially or in parallel based on the investigation results.

### When to Explore

- **Always** when there are 2+ plausible fix strategies
- **Skip** only when the fix is unambiguous (e.g., a missing case in a switch where the pattern is clear)

### Sequential (default)

Implement and test one strategy at a time. Present results before trying the next.

### Parallel (opt-in with `--parallel N`)

Launch parallel sub-agents in isolated worktrees. Each agent gets its own branch.
Worktrees are created under `.claude/worktrees/` in the repository root
(e.g., `.claude/worktrees/agent-01234567/`), with corresponding git branches.

**Warning**: Parallel agents use significant CPU/memory. On a laptop, 2-3 agents is practical.
On a remote server, up to 5.

### Common Fix Strategies

Strategies are listed best-first. See `slang-investigate` skill for the full ranking rationale.

| Bug Type | Best: IR pass | Acceptable: annotate/reject | Last resort: spot-fix |
|----------|---------------|----------------------------|----------------------|
| Missing validation | Add validation in IR pass | Reject at frontend (semantic) | Guard in emission |
| Wrong codegen | Fix or add the IR pass that transforms | Add new IR pass with annotations | Fix emission logic |
| ICE in pass | Transform input to handled form | Reject the input earlier (semantic/IR) | Handle missing case at crash site |
| Missing lowering | Extend existing lowering pass | Add new lowering pass | Emit diagnostic (unsupported) |

### Constructing the Agent Prompt

**Agent prompts must be self-contained.** Agents cannot read other skills.
The orchestrator must read the following skills and include their content
in each agent's prompt at the marked `{placeholder}` locations:
- `slang-build` → `{slang-build content}` (build commands, preset selection)
- `slang-run-tests` → `{slang-run-tests content}` (test commands, skip detection)
- `slang-write-test` → `{slang-write-test content}` (test syntax reference)
- `slang-create-issue` "Commit Rules" section → `{commit-rules}`

### Agent Prompt Template

```text
You are implementing and testing a fix strategy for a Slang compiler bug.
You are running in an isolated git worktree with your own branch.

## Bug Summary
{intake.md content}

## Root Cause
{investigation.md content}

## Your Strategy: [Strategy Name]
[Description of the specific fix strategy]

## Build Reference
{slang-build content}

## Test Syntax Reference
{slang-write-test content}

## Instructions

1. Implement the fix — minimal diff, follow existing patterns, comments only for non-obvious logic
2. Write a regression test in tests/bugs/ or tests/diagnostics/ or tests/language-feature/
3. Build: use build reference above
4. Validate: run regression test + original reproducer. Do NOT run full test suite — the orchestrator runs it on the winning strategy only
5. Format: ./extras/formatting.sh
6. Commit: {commit-rules}. Message: "Fix [short description]". Do NOT push.

## Report

STRATEGY: [Name]
BRANCH: [branch-name]
COMMIT: [commit-hash]
VERDICT: RECOMMENDED | VIABLE | RISKY | NOT_VIABLE
BUILD: pass | fail
REPRODUCER_FIXED: yes | no
REGRESSION_TEST: pass | fail (test_file: [path])
CHANGES: [files changed, functions modified, lines changed]
CORRECTNESS: [root cause fix / symptom fix / partial fix]
RISK: [low / medium / high — what could break]
CONCERNS: [any remaining concerns]
```

### Evaluation Output

Write `tmp/<bug-id>/alternatives.md` comparing strategies so the evaluation
trail is preserved. Also present the comparison in the conversation.

If a GitHub issue exists, post the alternatives summary as a comment (prefix
with `[Agent]`) so the analysis is visible to other contributors.

---

## Phase 4: PRESENT TO USER

**STOP and present the alternatives to the user.** Do not proceed without approval.

Present:
1. **Bug summary**: What's broken and why (2-3 sentences)
2. **Root cause**: The violated invariant and where it happens
3. **Alternatives**: Each strategy with verdict, scope, and risk
4. **Recommendation**: Which approach and why
5. **Ask**: Which strategy should be implemented?

---

## Phase 5: IMPLEMENT

After user approval, adopt the chosen strategy or implement fresh.

**Worktree CWD warning**: After agents complete, verify you are in the main repo
(`pwd` should be the project root, not `.claude/worktrees/`). Agent worktrees can
shift CWD if you read their files. Use absolute paths from the main repo root.

### Adopt from Worktree (if Phase 3 produced a passing implementation)

Do NOT cherry-pick — worktree branches share the same base commit, so cherry-pick
produces empty commits or conflicts. Instead, extract the diff and apply it:

```bash
git checkout -b fix-<bug-id>-<short-description>
git diff master..<winning-agent-branch> -- source/ tests/ | git apply
```

### Implement Fresh (if no clean result from Phase 3)

1. Create branch: `fix-<bug-id>-<short-description>`
2. Implement the fix — minimal diff, follow existing patterns
3. Write regression test (see `slang-write-test` skill)
4. Build and validate (see `slang-build` and `slang-run-tests` skills)

### Format, Commit, and PR

Follow commit rules from the `slang-create-issue` skill.

```bash
./extras/formatting.sh

git add <changed-files>
git commit -m "$(cat <<'EOF'
Fix [short description of what was broken]

[1-2 sentences explaining the root cause and the fix approach]
EOF
)"

git push -u origin HEAD
```

### Verify Pre-existing Failures

After the full test suite runs, if any tests fail, immediately run the same
failing tests on master to confirm they are pre-existing. Document this in the
PR body: "Tests X, Y, Z also fail on master (pre-existing)."

Create PR using the `slang-create-issue` skill PR format:
- Label: `pr: non-breaking` (default)
- Assignee: `--assignee @me`
- Link to issue: `Fixes #NNNN`
- Include: root cause, approach, alternatives considered, test plan
- Suggest reviewers based on `git log --format='%an' -- <changed-files> | sort | uniq -c | sort -rn`

---

## Anti-Patterns

### Investigation

1. **Skipping to a fix**: Do not propose a fix before understanding the root cause.
2. **Grep-and-patch**: Adding a null check without understanding why the null occurred.
3. **Single-strategy tunnel vision**: Always consider at least one alternative.
4. **Emission-layer fixes for IR problems**: Keep emission simple. Prefer IR passes.

### Fix

1. **Band-aid over root cause**: Special case in emission rather than fixing the IR pass.
2. **Overly broad changes**: Refactoring a subsystem when a targeted fix suffices.
3. **Missing regression test**: Every fix must include a test.
4. **Fix without validation**: Always rebuild and run the test suite.

---

## Decision Rules

### When to Fix vs File an Issue

| Signal | Action |
|--------|--------|
| Root cause clear, fix localized, confident in scope | Fix it |
| Root cause clear but fix touches many subsystems | File issue with analysis, fix if user approves |
| Root cause uncertain | File issue with investigation notes |
| Fix might break other things | File issue, propose fix, ask for review guidance |

---

## Output Structure

```text
tmp/<bug-id>/
├── intake.md              # Phase 1: symptom and reproducer
├── investigation.md       # Phase 2: root cause analysis (from slang-investigate)
├── alternatives.md        # Phase 3: fix strategies compared
└── SUMMARY.md             # Final result after fix is implemented

tests/                     # Regression test (committed)
└── <appropriate-dir>/regression-test.slang
```
