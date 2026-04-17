# Claude Multi-Agent Issue Solver

`extras/claude.sh` is a script that orchestrates multiple Claude Code agents to research, implement, and review a fix for a GitHub issue.

## Usage

```bash
extras/claude.sh <issue-number> [--repo <repo-name>]
```

- `issue-number`: GitHub issue number (required, digits only)
- `--repo`: Repository name under `shader-slang/` (default: `slang`)

```bash
# Fix issue #1234 in shader-slang/slang
extras/claude.sh 1234

# Explicitly specify the repo
extras/claude.sh 12345 --repo slang
```

Run from the repository root. The script requires `claude` and `jq` in PATH.

## How It Works

### Phase 1: Parallel Planning

Multiple Claude agents run in parallel, each researching the issue from a different angle:

| Angle | Focus |
|---|---|
| Root cause | Trace the bug from symptom to source |
| Test coverage | Identify tests that would catch the bug |
| Minimal change | Smallest diff with no architectural side effects |
| Architectural impact | Cross-file and cross-subsystem effects |
| Failure modes | Past wontfix/duplicate issues to avoid repeating |
| IR stage | Dump and bisect IR passes via `extras/split-ir-dump.md` to find which pass introduces the problem |
| Emit stage | `emit*.cpp/h` — code generation to HLSL/GLSL/SPIRV/Metal |
| AST stage | `slang-ast*.cpp/h`, `check*.cpp` — type-checking and name resolution |
| Core-module | `*.meta.slang` — built-in types, intrinsics, standard library |
| Module/link stage | Separate compilation, module serialization, IR linking |
| Legalization stage | `slang-ir-legalize*.cpp`, `slang-ir-lower*.cpp` — data representation transforms |
| Language syntax | Parser, spec, docs — incorrect or unimplemented syntax |
| Regression | `git log`/`git blame` — find the commit that introduced the issue |

Each agent writes its proposal to `issue-<N>/plan.<i>.md`.

### Phase 2: Synthesis

A single agent reads all proposals, identifies where they agree and diverge, and produces `issue-<N>/plan.best.md` — a synthesized plan with the strongest elements from each angle.

### Phase 3: Implementation and Commit

One agent reads `plan.best.md`, implements the fix, and commits the result.

### Phase 4: Virtual Review and Amend

A fresh agent reviews the commit as if it were a real code reviewer: checks correctness, style, edge cases, test coverage, and regressions. It also reviews all documentation changes (`CLAUDE.md`, `docs/user-guide/`, `docs/language-reference/`) and reverts additions that are not significant enough to justify a permanent change or are unrelated to the final implementation. All fixes are folded into the original commit via `git commit --amend`.

## Output

Plan files are written to `issue-<N>/` and are git-ignored. At the end the script prints the session ID so you can resume interactively:

```
To resume the implementation session interactively:
  claude --dangerously-skip-permissions --resume <session-id>
```

## Design Notes

**Why parallel planning agents?** Each agent has a fresh context window and a single focused angle, which avoids anchoring bias and surfaces root causes that a single agent might miss. Token cost is lower than it appears because planning agents only read — they do not write code.

**Why synthesize before implementing?** High-confidence areas (angles that agree) become the core of the plan. Divergence flags areas needing judgment, and the synthesis agent resolves them explicitly before any code is written.

**Why virtual review + amend instead of a separate commit?** It keeps the history clean — one logical change per issue — and the reviewer has full session context from the implementation, so it can reason about intent, not just diff.
