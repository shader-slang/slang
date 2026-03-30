---
name: slang-feature-test-flow
description: Orchestrator that researches a language feature, produces a test plan for user review, and fans out parallel agents that each deliver test branches with commits. Supports local-only (dry run) and full (PR + bug filing) modes.
---

# Feature Test Flow

**For**: End-to-end test coverage for a Slang language feature — from research through parallel test implementation to bug triage.

**Usage**: `/slang-feature-test-flow <feature-name> [--dry-run | --live] [--max-agents N] [--reference-url URL]`

- `--dry-run` (default): Local branches + local bug files, no PRs or GitHub issues filed
- `--live`: Push branches, create PRs, file GitHub issues
- `--max-agents N`: Maximum parallel agents (default 5)
- `--reference-url URL`: Additional documentation URL to fetch

---

## Phase 1: RESEARCH

Gather all available knowledge about the feature. This runs in the orchestrator (main conversation).

### Information Sources (priority order)

| Source | What to extract |
|--------|----------------|
| `--reference-url` (if provided) | Fetch and extract feature documentation |
| `external/spec/specification/` | Grammar, semantics, constraints, edge cases |
| `external/spec/proposals/` | Motivation, design decisions, known limitations |
| `docs/user-guide/` | User-facing behavior, examples, restrictions |
| DeepWiki (`mcp__deepwiki__ask_question`) | Implementation details, edge cases, cross-feature interactions |
| `source/slang/slang-diagnostics.lua` | Error codes related to the feature |
| `tests/` | Existing test inventory + coverage assessment |
| `source/slang/` | Key implementation files, TODO/FIXME gaps |

### Research Steps

1. **Check for spec repo**: If `external/spec/` doesn't exist, ask user whether to clone it
2. **Fetch reference URL** if provided (use WebFetch)
3. **Search spec and proposals** for the feature keyword
4. **Search user guide** chapters
5. **Query DeepWiki** for implementation details and edge cases
6. **Enumerate ALL diagnostics** (see below)
7. **Inventory existing tests**: find all tests related to the feature, categorize what they cover
8. **Search compiler source** for TODO/FIXME related to the feature

### Exhaustive Diagnostic Enumeration (mandatory)

Do NOT rely on keyword search of documentation alone. Extract ALL
diagnostic codes related to the feature directly from compiler source:

```bash
# Search diagnostic definition files with multiple keywords
# Include the feature name, related concepts, and synonyms
rg -i "generic|constraint|speciali|conform|type.param|pack|where.clause" \
  source/slang/slang-diagnostic*.h --context 2
```

For each code found:
1. Record: code number, diagnostic name, message text
2. Search `tests/` to check if any test triggers this code
3. Mark as COVERED or UNCOVERED

This produces the ground truth for error-path coverage. Keyword search
of docs misses codes that use different terminology -- the generics
feature had 22 diagnostic codes (out of 57 total) that were missed by
doc-based keyword search.

### Line/Branch Coverage Baseline (optional)

Check the nightly coverage report at
`https://shader-slang.org/slang-coverage-reports/reports/latest/linux/index.html`
for key source files related to the feature. Record baseline coverage
numbers in research.md and use low branch coverage as a signal for
untested error paths. Do not use coverage % as a test-writing target.

### Output

Write `tmp/<feature>/research.md`:
- Feature overview (from spec + docs)
- Complete list of behaviors/constraints/error conditions
- **Complete diagnostic code table** with COVERED/UNCOVERED status
- Coverage baseline for key source files (if checked)
- Existing test inventory with coverage assessment
- Known limitations or open issues

---

## Phase 2: PLAN

Decompose the feature into **semantic dimensions** — each becomes an independent sub-plan that one agent can implement as one focused branch.

### Decomposition Strategy

Split by semantic area, NOT by test type:
```text
Feature: generics
├── Sub-plan A: Type parameters on structs
│   ├── Positive: basic usage, multiple params, nested
│   ├── Negative: constraint violations, recursive types
│   └── Targets: cpu (+ GPU targets for CI)
├── Sub-plan B: Type parameters on functions
├── Sub-plan C: Where clauses and constraints
├── Sub-plan D: Generic interfaces and conformance
└── Sub-plan E: Specialization and cross-feature interactions
```

**Why semantic dimensions?**
- Naturally independent → parallelizable with no conflicts
- Each covers positive + negative + edge within its scope
- **Each becomes one focused, reviewable branch/PR** — one sub-plan = one agent = one branch = one PR
- Keep sub-plans small enough for a single reviewable PR (aim for 5-8 test files, ~100-200 lines)
- Related sub-plans can be combined into a single PR if the total stays under ~500 lines (e.g., all diagnostic tests for a feature, or all functional tests). Use judgement to group by theme.

**Negative testing requirement**: Every sub-plan that includes positive
functional tests for constrained features (interface conformance, where
clauses, generic constraints, typealias constraints) MUST also include
companion negative diagnostic tests that verify the constraints are
enforced. A positive-only test for a constrained feature will be flagged
in code review. Plan the negative companion at sub-plan time, not as an
afterthought.

### Sub-plan Format

Write each to `tmp/<feature>/sub-plans/sub-plan-<letter>.md`:

```markdown
# Sub-plan [Letter]: [Name]

## Branch Name
<feature>-<dimension>

## Title
Add tests for [feature]: [dimension]

## Context
[2-3 sentences from spec about this dimension]

## Spec References
- Section X.Y: [relevant quote/summary]

## Existing Coverage
- [existing tests and what they cover]
- [identified gaps]

## Tests to Write

### 1. feature-scenario.slang
- **Path**: tests/language-feature/<feature>/<name>.slang
- **Type**: compute | diagnostic | interpret
- **Validates**: [specific behavior from spec]
- **Key assertion**: [what the CHECK verifies]
- **Negative companion**: [yes/no — if yes, list the -negative.slang file]
- **Priority**: HIGH | MEDIUM | LOW
- **Score**: N/10

### 2. ...

## Anti-overlap
- Do NOT test [X] — covered by sub-plan [other]

## Overlap with Existing Tests
- Checked against: [list files searched]
- No significant overlap / Extending [file] instead of creating new
```

### Gap Traceability (mandatory)

Every gap from research.md must map to either a sub-plan or an explicit
SKIP with reason. No gap may be silently dropped.

Write `tmp/<feature>/gap-traceability.md`:

```markdown
| Gap | Action | Sub-plan | Reason |
|-----|--------|----------|--------|
| 30400 generic-type-needs-args | WRITE | A | No test exists |
| 30404 invalid-equality-constraint | SKIP | — | Already tested in existing file |
| Coercion constraints | SKIP | — | Low priority, score 3/10 |
| Constructor type inference | SKIP | — | Feature not implemented |
```

Present this table alongside the plan for user review. If any research
gap is not accounted for, the plan is incomplete.

### Plan Summary

Write `tmp/<feature>/plan.md` with overview table:

| Sub-plan | Dimension | Tests | Priority |
|----------|-----------|-------|----------|
| A | Type params on structs | 5 | HIGH |
| B | Type params on functions | 4 | HIGH |
| ... | | | |

### User Review Gate

**STOP HERE and present the plan to the user.** The user can:
- Approve all sub-plans
- Drop specific sub-plans
- Adjust scope (add/remove tests)
- Merge sub-plans that are too small

---

## Phase 3: EXECUTE

Launch parallel agents (one per approved sub-plan) in **worktree isolation** using
`subagent_type="best-of-n-runner"`. Each agent gets its own git worktree and branch,
so agents cannot conflict with each other even when creating files in the same
directory hierarchy.

### Agent Configuration

Each agent receives:
1. Its sub-plan document
2. The `slang-test-development` skill content (canonical test syntax reference)
3. Feature context from research.md
4. Mode flag (dry-run vs live)

Launch agents in parallel by sending multiple `Task` tool calls in a single message,
each with `subagent_type="best-of-n-runner"`.

### Agent Prompt

```text
You are implementing tests for the Slang compiler.
You are running in an isolated git worktree with your own branch.

## Your Sub-plan
{sub-plan content}

## Test Syntax Reference
{slang-test-development SKILL.md content}

## Feature Context
{research.md overview section}

## Project Context

This is the Slang shading language compiler (C++). Key conventions:
- Build: cmake --build --preset release --target slangc slang-test >/dev/null 2>&1 || \
    cmake --build --preset release --target slangc slang-test
- Test: ./build/Release/bin/slang-test tests/path/to/test.slang (run from repo root)
- Format: ./extras/formatting.sh
- Single-dash CLI options: -target spirv (not --target)
- Do not mention AI tools in commits
- No trailing whitespace or blank lines with only spaces/tabs

## Instructions

### Step 0: Build (if needed)
Check if ./build/Release/bin/slang-test exists. If not, build:
  cmake --preset default
  cmake --build --preset release --target slangc slang-test >/dev/null 2>&1 || \
    cmake --build --preset release --target slangc slang-test

### Step 1: Write and validate tests
For each test in the sub-plan:
  a. Create the .slang test file at the specified path
  b. Follow the test templates from the syntax reference
  c. Write natural comments explaining semantic behavior
  d. Run the test: ./build/Release/bin/slang-test tests/path/to/test.slang
  e. If it fails:
     - Wrong expected value → fix the test
     - Compiler bug → record bug in structured format (see below)
       For DISABLED tests: rename the //TEST directive to //DISABLE_TEST

### Step 2: Pre-commit quality checks
Before formatting or committing, verify EVERY test file:
  a. Filename matches what the test actually verifies
  b. All comments reference the correct interfaces, error codes, and
     behavior (cross-check against actual code in the file)
  c. No dead code: every declared function/struct/variable is used
  d. No duplicates: search existing tests for the same scenario
     (rg "keyword" tests/language-feature/<feature>/)
  e. Feature support: for functional tests, confirm the feature compiles
     with a quick slangc invocation before writing the full test
  f. DIAGNOSTIC_TEST uses exhaustive mode unless there is a documented
     reason for non-exhaustive; never use non-exhaustive "just in case"
  g. Negative companion: if any functional test exercises a constrained
     feature (interface conformance, where clause, generic constraint),
     verify a companion -negative.slang diagnostic test exists that
     proves the constraint is enforced by rejecting invalid types

### Step 3: Format
Run ./extras/formatting.sh on changed files.

### Step 4: Create branch & commit
Create a dedicated branch from master for this sub-plan:
  git checkout -b <branch-name>
Stage all new test files. Create a commit with message:
  "Add tests for <feature>: <dimension>"
Do NOT mention AI tools in the commit message.

**IMPORTANT**: Each agent creates its own branch and PR independently.
The orchestrator does NOT merge agents' work into a single branch/PR.
One sub-plan = one branch = one PR. This keeps PRs focused and reviewable.

### Step 5: Mode-dependent actions
**Dry-run mode (default)**:
- Do NOT push or create PRs
- The branch stays local in the worktree
- The orchestrator will collect results and can copy files to the main repo later

**Live mode**:
- Push branch with -u flag
- Create PR using gh pr create with:
  - Title: {title from sub-plan}
  - Label: pr: non-breaking
  - Assignee: @me
  - Body: Summary of tests added + what they cover
- Each agent is responsible for creating its own PR — do not defer to the orchestrator

### Step 6: Report back
Return a structured result:

BRANCH: <branch-name>
COMMIT: <commit-hash>
PR_URL: <url or "dry-run: not created">

TESTS:
- name: <test-file-path>
  status: pass | fail | disabled
  notes: <any relevant detail>

BUGS:
- id: <sub-plan-letter>-<number> (e.g. B-1)
  test: <test-file-path that triggered it>
  symptom: <what happened — crash, wrong output, missing error>
  error_output: <exact compiler output, truncated to key lines>
  reproducer: |
    <minimal .slang code that triggers the bug>
  target: <which target(s) affected>
  severity: ICE | wrong-codegen | missing-diagnostic | validation-error
```

### Constraints
- Max 5 parallel agents (configurable via --max-agents)
- Each agent uses `subagent_type="best-of-n-runner"` for git worktree isolation
- Plan phase assigns unique branch names and file paths
- Agents run fully autonomously: write tests, build, run, format, commit
- In dry-run mode agents stop at commit; in live mode they push and create PRs

---

## Phase 4: BUG TRIAGE

After all agents complete, collect and triage all reported bugs.

### Step 1: Collect
Gather all bug reports from all agents into `tmp/<feature>/bugs/`.

### Step 2: Deduplicate
Group bugs by:
1. Same error code → likely same root cause
2. Same crash location / ICE message
3. Same symptom on same target

For each group, keep one canonical bug report (best reproducer, clearest symptom).

### Step 3: Write bug files

For each unique bug, write `tmp/<feature>/bugs/bug-<id>.md`:

```markdown
# Bug <ID>: [Short title]

## Severity
ICE | wrong-codegen | missing-diagnostic | validation-error

## Reproducer
\`\`\`hlsl
[minimal code]
\`\`\`

**Command:**
\`\`\`bash
slangc -target [target] test.slang
\`\`\`

## Expected Behavior
[What the spec/docs say should happen]

## Actual Behavior
[Error output]

## Affected Tests
- `tests/path/to/disabled-test.slang` (disabled in branch `<feature>-<dim>`)

## Duplicates
- Sub-plan A, bug A-1: [same/different]
- Existing GitHub issue: [#NNNN or "not found"]
```

### Step 4: Mode-dependent actions

**Dry-run mode**: Stop here. Bug files in `tmp/<feature>/bugs/` are the deliverable.

**Live mode**:
- Search existing GitHub issues for each bug
- File new issues for confirmed new bugs using `slang-issues` bug report format with `--assignee @me`
- Comment on existing issues with new reproducers
- Update PR descriptions to link to filed issues

---

## Phase 5: REPORT

Write `tmp/<feature>/SUMMARY.md` and present to user:

```markdown
## Feature Test Flow: [feature] — Results

### Mode
dry-run | live

### Branches Created
| # | Sub-plan | Branch | Tests | Pass | Disabled | Bugs |
|---|----------|--------|-------|------|----------|------|
| A | Type params on structs | generics-struct-params | 5 | 5 | 0 | 0 |
| B | Type params on functions | generics-func-params | 4 | 3 | 1 | 1 |
| ...

### PRs Created (live mode only)
| Sub-plan | PR |
|----------|-----|
| A | #1234 |
| ...

### Bugs Found
| Bug ID | Severity | File | Description |
|--------|----------|------|-------------|
| B-1 | ICE | tmp/generics/bugs/bug-B-1.md | Crash when inferring... |

### Bugs Skipped (duplicates)
| Bug ID | Reason |
|--------|--------|
| D-2 | Same as B-1 (same error code) |

### Remaining Gaps
- [anything from the plan that couldn't be tested]

### Next Steps
- [ ] Review branch generics-struct-params
- [ ] Review branch generics-func-params
- [ ] Investigate bug B-1
```

---

## Output Structure

```text
tmp/<feature>/
├── research.md                    # Phase 1 output
├── plan.md                        # Phase 2 overview
├── sub-plans/
│   ├── sub-plan-a.md
│   ├── sub-plan-b.md
│   └── ...
├── bugs/
│   ├── bug-A-1.md
│   ├── bug-B-1.md
│   └── ...
├── agent-results/
│   ├── result-a.md                # Raw agent output
│   ├── result-b.md
│   └── ...
└── SUMMARY.md                     # Phase 5 final report
```
