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

Write `tmp/<issue-repository>-<bug-id>/intake.md`:

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

### Agent Instructions

Each agent receives: intake.md, investigation.md, the fix strategy to implement, and references
to the `slang-build`, `slang-run-tests`, and `slang-write-test` skills.

Each agent must:
1. Implement the fix (minimal diff, follow existing patterns)
2. Write a regression test (see `slang-write-test` skill)
3. Build and validate (see `slang-build` and `slang-run-tests` skills)
4. Format: `./extras/formatting.sh`
5. Commit (do NOT push): `"Fix [short description]"` — follow commit rules from `slang-create-issue` skill.
6. Report: VERDICT, build/test results, changes, risk assessment

### Evaluation Output

After all strategies are evaluated, write `tmp/<bug-id>/alternatives.md` comparing
build success, reproducer fixed, test suite regressions, root cause vs symptom fix, risk.

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

### Adopt from Worktree (if Phase 3 produced a passing implementation)

```bash
git checkout -b fix-<bug-id>-<short-description>
git cherry-pick <commit-hash-from-winning-strategy>
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
