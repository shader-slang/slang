---
name: slang-bug-fix
description: Analyze a Slang compiler bug from a test failure, CI log, or GitHub issue, investigate root causes in depth, explore alternative fixes in parallel, present options to the user, and implement the chosen fix with tests and a PR.
---

# Slang Bug Fix

**For**: Diagnosing and fixing Slang compiler bugs -- from symptom to root cause to validated fix.

**Core Principle**: Dig into the root cause. Never apply a band-aid fix without understanding
why the bug exists and what invariants are violated. Present alternative fix strategies to the
user before implementing.

**Usage**: `/slang-bug-fix <bug-source>`

Where `<bug-source>` is one of:
- A GitHub issue number or URL (e.g., `#10419` or `https://github.com/shader-slang/slang/issues/10419`)
- A test file path (e.g., `tests/bugs/my-failing-test.slang`)
- A CI log URL or terminal output
- A description of the symptom

---

## Phase 1: INTAKE -- Understand the Symptom

Gather all available information about the bug from the provided source.

### From a GitHub Issue

```bash
gh issue view <number> --repo shader-slang/slang --json title,body,labels,comments
```

Extract:
- Reproducer code (mandatory -- if missing, ask the user)
- Target(s) affected
- Error message or crash output
- Expected vs actual behavior

### From a CI Log

Parse the log to extract:
- The failing test name and path
- The exact error output (error code, ICE message, assertion, segfault)
- The command that was run
- The target and platform

### From a Local Test Failure

```bash
./build/RelWithDebInfo/bin/slang-test tests/path/to/test.slang
```

Capture:
- Full test output
- Exit code
- Any assertion or ICE messages

### From a Description

Ask the user for:
- A minimal reproducer (code + command)
- The target(s) affected
- The error output

### Intake Output

Write `tmp/<bug-id>/intake.md`:

```markdown
# Bug Intake: [short title]

## Source
[GitHub issue #N / CI log / local test / user description]

## Reproducer
\`\`\`hlsl
[minimal code]
\`\`\`

**Command:**
\`\`\`bash
slangc -target [target] test.slang
\`\`\`

## Error Output
[exact error message, crash output, or wrong behavior]

## Expected Behavior
[what should happen]

## Affected Targets
[spirv, hlsl, cuda, cpu, metal, wgsl, all]

## Initial Classification
- [ ] ICE (Internal Compiler Error / error 99999)
- [ ] Wrong codegen (compiles but produces incorrect output)
- [ ] Missing diagnostic (should reject but silently accepts or gives wrong error)
- [ ] SPIRV validation error
- [ ] Crash / segfault / assertion failure
- [ ] Performance regression
```

---

## Phase 2: ROOT CAUSE INVESTIGATION

This is the most important phase. Do not skip to a fix.

### Step 1: Reproduce Locally

Confirm the bug reproduces with the current build:

```bash
cmake --build --preset releaseWithDebugInfo --target slangc >/dev/null 2>&1 || cmake --build --preset releaseWithDebugInfo --target slangc

# For ICE/crash
./build/RelWithDebInfo/bin/slangc -target <target> test.slang

# For SPIRV validation
SLANG_RUN_SPIRV_VALIDATION=1 ./build/RelWithDebInfo/bin/slangc -target spirv test.slang

# For wrong codegen (compute test)
./build/RelWithDebInfo/bin/slang-test tests/path/to/test.slang
```

If the bug does not reproduce, document why and stop.

### Step 2: Classify the Error

| Error Type | What to Look For | Investigation Strategy |
|------------|------------------|----------------------|
| ICE 99999 | `SLANG_UNIMPLEMENTED_X`, exception, `SLANG_UNEXPECTED` | Find the exact crash site via error message text search |
| Assertion | `SLANG_ASSERT`, `assert failure:` | Search for the assertion text in source |
| Segfault | No error, process killed | Use `-dump-ir` to find the last pass before crash |
| Wrong codegen | Output differs from expected | Compare IR at different pass stages |
| Missing diagnostic | No error emitted for invalid code | Check semantic checker for the relevant validation |
| SPIRV validation | `spirv-val` error message | Compare SPIRV output with `-emit-spirv-via-glsl` reference |

### Step 3: Locate the Crash Site

For ICE/assertion/crash, find the exact source location:

```bash
# Search for the error message text
rg "error message text" source/slang/ source/compiler-core/

# For SLANG_UNIMPLEMENTED_X crashes
rg "SLANG_UNIMPLEMENTED_X" source/slang/ --context 5

# For assertion failures, search the assertion text
rg "assert.*message text" source/slang/
```

### Step 4: Trace the Code Path

Use IR dumps to understand which pass is involved:

```bash
# Dump IR before/after specific passes
./build/RelWithDebInfo/bin/slangc -dump-ir -target <target> -o /dev/null test.slang 2>&1 | \
  python extras/split-ir-dump.py

# Dump IR around the suspected pass
./build/RelWithDebInfo/bin/slangc \
  -dump-ir-before <pass-name> \
  -dump-ir-after <pass-name> \
  -target <target> -o /dev/null test.slang > pass-dump.txt 2>&1
```

### Step 5: Understand the Design Context

Before proposing any fix, answer these questions:

1. **Which compiler stage owns this behavior?**
   - Frontend (lexer, parser, semantic checker)
   - IR generation (lowering from AST to IR)
   - IR passes (transformation, optimization, legalization)
   - Code emission (target-specific output)

2. **What invariant is violated?**
   - A type that should have been rejected reached a pass that cannot handle it
   - A transformation produced invalid IR
   - An instruction was not lowered before reaching emission
   - A layout calculation produced incorrect results

3. **Is this the right layer to fix?**
   The Slang compiler philosophy: keep emission simple, do heavy lifting in IR passes.
   If the fix is in the emitter, ask whether an IR pass should have handled it first.

4. **What related code paths exist?**
   - Does a similar construct work on other targets?
   - Does a similar type/pattern work in the same context?
   - Is there existing validation for a related case?

5. **Could this be intentional?**
   Some behaviors that look like bugs may be deliberate (HLSL compatibility,
   performance trade-offs, known limitations documented in issues).

### Step 6: Search for Related Issues and Prior Art

```bash
# Search for related issues
gh issue list --repo shader-slang/slang --search "error message keywords" --limit 10

# Search for related fixes in git history
git log --all --oneline --grep="error message keywords" -- source/slang/

# Check if there's a TODO/FIXME near the crash site
rg "TODO|FIXME|HACK|WORKAROUND" source/slang/<file>.cpp --context 3
```

### Investigation Output

Write `tmp/<bug-id>/investigation.md`:

```markdown
# Root Cause Investigation

## Crash Site
- File: `source/slang/<file>.cpp`
- Function: `functionName()`
- Line: N
- Error: [exact error message or assertion]

## Code Path
[How the input reaches the crash site -- which passes transform it, what decisions lead here]

## Violated Invariant
[What assumption is broken and why]

## Design Context
- Stage: [frontend / IR pass / emission]
- Right layer: [yes/no -- if no, which layer should handle it]
- Related patterns: [similar constructs that work, and why they work]

## Related Issues
- #NNNN: [relationship]
- Prior fix in commit abc123: [what it did]

## Potential Fix Locations
1. [Location 1]: [brief description of what a fix here would look like]
2. [Location 2]: [brief description]
3. [Location 3]: [brief description]
```

---

## Phase 3: EXPLORE ALTERNATIVES

Launch parallel sub-agents in isolated worktrees to implement and test different fix
strategies. Each agent gets its own branch and working directory, implements the fix,
builds, runs the reproducer and test suite, and reports back with concrete results.

### When to Use Parallel Exploration

- **Always** when there are 2+ plausible fix locations or strategies
- **Skip** only when the fix is unambiguous (e.g., a missing case in a switch statement
  where the pattern for other cases is clear)

### Agent Configuration

Launch each agent with `subagent_type="best-of-n-runner"`. Each agent runs in its own
git worktree with an isolated branch, so agents cannot conflict with each other.

### Agent Prompt Template

```text
You are implementing and testing a fix strategy for a Slang compiler bug.
You are running in an isolated git worktree with your own branch.

## Bug Summary
{intake.md content}

## Root Cause
{investigation.md content}

## Your Strategy: [Strategy Name]

Implement the following fix approach:
[Description of the specific fix strategy]

## Project Context

This is the Slang shading language compiler (C++). Key conventions:
- Build: cmake --build --preset releaseWithDebugInfo --target slangc slang-test
- Test: ./build/RelWithDebInfo/bin/slang-test tests/path/to/test.slang
- Format: ./extras/formatting.sh
- Single-dash CLI options: -target spirv (not --target)
- SPIRV validation: SLANG_RUN_SPIRV_VALIDATION=1 slangc -target spirv test.slang
- Redirect build output: cmake --build ... >/dev/null 2>&1 || cmake --build ...
- Do not mention AI tools in commits

## Instructions

### Step 1: Implement the fix
1. Read the relevant source files to understand the current implementation
2. Implement the fix -- change only what is necessary
3. Follow existing patterns in the codebase
4. Add comments only for non-obvious logic

### Step 2: Write a regression test
1. Create a .slang test file that reproduces the original bug
2. Place it in tests/bugs/ or tests/diagnostics/ or tests/language-feature/
3. Use the appropriate test type:
   - COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
   - DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
   - SIMPLE(filecheck=CHECK): -target spirv

### Step 3: Build and validate
1. Build: cmake --build --preset releaseWithDebugInfo --target slangc slang-test >/dev/null 2>&1 || \
     cmake --build --preset releaseWithDebugInfo --target slangc slang-test
2. Run the regression test: ./build/RelWithDebInfo/bin/slang-test tests/path/to/test.slang
3. Run the original reproducer to confirm it's fixed
4. Run SPIRV validation if applicable
5. Run broader test suite: ./build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 8
6. Record pass/fail counts

### Step 4: Format
Run ./extras/formatting.sh on changed files.

### Step 5: Commit
Create a commit with message: "Fix [short description]"
Do NOT push -- the orchestrator will compare results across strategies.

### Step 6: Report back
Return a structured result with ALL of the following:

STRATEGY: [Name]
BRANCH: [branch-name]
COMMIT: [commit-hash]
VERDICT: RECOMMENDED | VIABLE | RISKY | NOT_VIABLE

BUILD: pass | fail
  build_output: [key lines if failed]

REPRODUCER_FIXED: yes | no
  output: [compiler output after fix]

REGRESSION_TEST: pass | fail
  test_file: [path]
  output: [test runner output]

TEST_SUITE: pass | fail
  total: N
  passed: N
  failed: N
  failed_tests: [list of any newly failing tests]

CHANGES:
- file: source/slang/<file>.cpp
  function: <name>
  description: <what changes>
  lines_changed: N

CORRECTNESS: [root cause fix / symptom fix / partial fix]
COMPLETENESS: [handles all cases / handles reproducer only / handles N of M cases]
RISK: [low / medium / high -- what could break]

EDGE_CASES:
- [case 1]: [how this strategy handles it]

CONCERNS:
- [any remaining concerns or unknowns]
```

### Common Fix Strategies to Explore

Choose 2-4 of these based on the bug classification:

| Bug Type | Strategy A | Strategy B | Strategy C |
|----------|------------|------------|------------|
| Missing validation | Add check at frontend (semantic) | Add check at IR pass | Add check at emission |
| Wrong codegen | Fix the IR pass that transforms | Fix the emission logic | Add a new IR pass |
| ICE in pass | Handle the missing case in the pass | Reject the input earlier | Transform to handled form |
| Missing lowering | Extend existing pass | Add new pass | Emit diagnostic (unsupported) |

### Evaluation Output

After all agents return, compare the **real results** (not just analysis). Write
`tmp/<bug-id>/alternatives.md`:

```markdown
# Fix Alternatives

## Strategy A: [Name]
- **Branch**: [branch-name]
- **Verdict**: [RECOMMENDED / VIABLE / RISKY / NOT_VIABLE]
- **Build**: pass/fail
- **Reproducer fixed**: yes/no
- **Regression test**: pass/fail
- **Test suite**: N passed, M failed (list any new failures)
- **Approach**: [1-2 sentences]
- **Pros**: [bullet points]
- **Cons**: [bullet points]
- **Scope**: [N files, ~M lines]
- **Risk**: [low / medium / high]

## Strategy B: [Name]
...

## Comparison Matrix

| Criterion | Strategy A | Strategy B | Strategy C |
|-----------|-----------|-----------|-----------|
| Build succeeds | yes/no | yes/no | yes/no |
| Reproducer fixed | yes/no | yes/no | yes/no |
| Test suite regressions | 0 | N | ... |
| Fixes root cause | yes/no | yes/no | yes/no |
| Handles all cases | yes/partial/no | ... | ... |
| Risk level | low/med/high | ... | ... |
| Code scope | N files | ... | ... |
| Follows existing patterns | yes/no | ... | ... |

## Recommendation
[Which strategy and why, backed by concrete build/test evidence.]
```

---

## Phase 4: PRESENT TO USER

**STOP and present the alternatives to the user.** Do not proceed to implementation
without user approval.

### Presentation Format

Present a concise summary:

1. **Bug summary**: What's broken and why (2-3 sentences)
2. **Root cause**: The violated invariant and where it happens
3. **Alternatives**: Each strategy with verdict, scope, and risk
4. **Recommendation**: Which approach and why
5. **Ask**: Which strategy should be implemented?

The user may:
- Approve the recommendation
- Choose a different strategy
- Ask for more investigation
- Provide additional context that changes the analysis
- Decide not to fix (e.g., known limitation, low priority)

---

## Phase 5: IMPLEMENT

After user approval, adopt the chosen strategy's worktree branch or implement fresh.

### Option A: Adopt from Worktree (preferred)

If Phase 3 produced a passing implementation in an isolated worktree:

```bash
# The best-of-n-runner agent already committed to its branch.
# Cherry-pick or merge the winning branch into a clean fix branch.
git checkout -b fix-<bug-id>-<short-description>
git cherry-pick <commit-hash-from-winning-strategy>

# If the user wants adjustments, make them now
# Rebuild and re-validate after any changes
cmake --build --preset releaseWithDebugInfo --target slangc slang-test >/dev/null 2>&1 || \
  cmake --build --preset releaseWithDebugInfo --target slangc slang-test
./build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 8
```

### Option B: Implement Fresh

If no Phase 3 agent produced a clean result, or if the user wants a different approach:

1. **Create branch**: `git checkout -b fix-<bug-id>-<short-description>`
2. **Implement the fix**:
   - Minimal diff -- change only what is necessary
   - Follow existing patterns in the codebase
   - Add comments only for non-obvious logic
   - No trailing whitespace, no lines with only spaces/tabs
3. **Write regression test** (use `slang-test-development` skill for syntax reference):
   - Place in `tests/bugs/`, `tests/diagnostics/`, or `tests/language-feature/`
   - Must reproduce the original bug without the fix
   - Must pass with the fix
   - Must verify specific behavior, not just "doesn't crash"
4. **Build and validate**:
   ```bash
   cmake --build --preset releaseWithDebugInfo --target slangc slang-test >/dev/null 2>&1 || \
     cmake --build --preset releaseWithDebugInfo --target slangc slang-test
   ./build/RelWithDebInfo/bin/slang-test tests/path/to/regression-test.slang
   SLANG_RUN_SPIRV_VALIDATION=1 ./build/RelWithDebInfo/bin/slangc -target spirv test.slang
   ./build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 8
   ```

### Format

```bash
./extras/formatting.sh
```

### Commit and PR

Follow the project's PR conventions from the `slang-issues` skill:

- **Commit message**: Describe the fix concisely. Do not mention AI tools.
- **PR label**: `pr: non-breaking` (default) or `pr: breaking`
- **PR assignee**: `--assignee @me`
- **PR body**: Include root cause analysis, approach taken, alternatives considered,
  and test plan. Do not narrate the diff line by line.
- **Link to issue**: `Fixes #NNNN` if a GitHub issue exists.

```bash
# Format before committing
./extras/formatting.sh

# Stage and commit
git add <changed-files>
git commit -m "$(cat <<'EOF'
Fix [short description of what was broken]

[1-2 sentences explaining the root cause and the fix approach]

Fixes #NNNN
EOF
)"

# Push and create PR
git push -u origin HEAD
gh pr create \
  --title "Fix [short description]" \
  --label "pr: non-breaking" \
  --assignee @me \
  --body "$(cat <<'EOF'
## Summary
- [what this PR fixes]

## Motivation
[link to issue, description of the bug impact]

## Technical Details
[root cause, fix approach, why this strategy was chosen over alternatives]

## Test Plan
- [ ] New regression test: `tests/path/to/test.slang`
- [ ] Original reproducer compiles correctly
- [ ] Existing tests pass
- [ ] SPIRV validation (if applicable)

EOF
)"
```

---

## Anti-Patterns

### Investigation Anti-Patterns

1. **Skipping to a fix**: Do not propose a fix before understanding the root cause.
   The first plausible fix is often wrong or incomplete.

2. **Grep-and-patch**: Finding the crash site and adding a null check or early return
   without understanding why the null/unexpected state occurred.

3. **Single-strategy tunnel vision**: Do not commit to the first fix idea. Always
   consider at least one alternative.

4. **Ignoring related code**: If a similar construct works on another target or in
   a different context, understanding why is critical.

5. **Emission-layer fixes for IR problems**: The Slang philosophy is to keep emission
   simple. If you're adding complexity to an emitter, ask whether an IR pass should
   handle the transformation instead.

### Fix Anti-Patterns

1. **Band-aid over root cause**: Adding a special case in emission rather than fixing
   the IR pass that should have handled it.

2. **Overly broad changes**: Refactoring a subsystem when a targeted fix suffices.

3. **Missing regression test**: Every fix must include a test that would have caught
   the bug.

4. **Fix without validation**: Always rebuild and run the test suite after the fix.

5. **Ignoring edge cases**: If the fix handles the reproducer but not related patterns,
   document the remaining gaps.

---

## Decision Rules

### When to Add Validation vs Implement Support

| Signal | Action |
|--------|--------|
| The construct is fundamentally unsupported by the architecture | Add diagnostic rejecting it |
| Similar constructs work, this one was missed | Extend existing support |
| The spec says it should work | Implement support |
| The spec is silent | Investigate intent, ask user |

### When to Fix vs File an Issue

| Signal | Action |
|--------|--------|
| Root cause is clear, fix is localized, confident in scope | Fix it |
| Root cause is clear but fix touches many subsystems | File issue with analysis, fix if user approves |
| Root cause is uncertain | File issue with investigation notes |
| Fix might break other things | File issue, propose fix, ask for review guidance |

### Confidence Levels for Fix Proposals

| Level | When | Recommendation |
|-------|------|----------------|
| **High** | Single crash site, clear pattern, similar fixes exist | Implement directly |
| **Medium** | Root cause identified but multiple interaction points | Present alternatives, ask user |
| **Low** | Symptom clear but root cause uncertain | Investigate more or file issue with findings |

---

## Output Structure

```text
tmp/<bug-id>/
├── intake.md              # Phase 1: symptom and reproducer
├── investigation.md       # Phase 2: root cause analysis
├── alternatives.md        # Phase 3: fix strategies compared
├── test-repro.slang       # Minimal reproducer used during investigation
└── SUMMARY.md             # Final result after fix is implemented

tests/bugs/                # or tests/diagnostics/ or tests/language-feature/
└── regression-test.slang  # Regression test (committed)
```

**Note**: The `tmp/` directory is gitignored and for working notes only. Only the
fix code and regression test under `tests/` should be committed.
