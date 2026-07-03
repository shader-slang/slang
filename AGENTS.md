<!--
SPDX-FileCopyrightText: The Khronos Group, Inc.
SPDX-License-Identifier: CC-BY-4.0
-->

# Repository Guidelines

## Project Structure & Module Organization

Slang is a shading-language compiler and runtime implemented primarily in C++20 and built with
CMake.

Key directories:

- `source/`: core implementation, including `source/slang/`, `source/core/`,
  `source/compiler-core/`, and tools like `source/slangc/`.
- `include/`: public API headers.
- `prelude/` and `source/standard-modules/`: standard/prelude headers.
- `tests/`: test suites grouped by feature or target.
- `tools/`: test infrastructure and developer tools.
- `docs/`: documentation.
- `examples/`: runnable samples.
- `cmake/`: CMake helpers.
- `external/`: vendored dependencies.

## Repository-Local Skills

This repository stores local agent skills under `.claude/skills/`. Codex and other non-Claude
harnesses should still consult those `SKILL.md` files when a user asks for the workflow they
describe.

Review-related skills:

- `slang-review-clarity-workflow`: coordinate the end-to-end clarity review workflow.
- `slang-review-clarity`: generate high-level clarity and explainability review candidates.
- `slang-review-fine-grained-clarity`: generate line-by-line name/comment/type/function
  consistency review candidates.
- `slang-review-consolidate-candidates`: merge candidate files and resolve duplicates,
  overlap, and superseded comments.
- `slang-review-scope-filter`: conservatively filter candidate comments to issues the PR
  author can reasonably own before posting.
- `slang-review-resolve-judgment-calls`: resolve uncertain candidates with focused follow-up
  analysis before posting.
- `slang-review-post-github`: post filtered candidates as one proper GitHub PR review.

## WSL and Windows Tooling

When working in this repository from WSL on Windows, use Windows-native developer tools by
default unless the user explicitly asks for the WSL/Linux version.

- Use `git.exe`, not bare `git`. These worktrees use Windows path conventions; WSL Git can
  corrupt or misinterpret worktree state, and Windows Git has much better file I/O performance
  on this checkout.
- When the `slang-build` skill invokes CMake, use `cmake.exe`, not bare `cmake`, for
  Windows-hosted configure and build commands. Windows CMake can find Visual Studio 2026 and
  the Windows toolchains required by the `vs2026` preset.
- Use `gh.exe` instead of bare `gh` when GitHub CLI commands need to share the same
  Windows-native Git and credential context.
- Convert WSL paths before passing them to Windows tools, for example `wslpath -w "$path"`.
  Convert paths printed by Windows tools back before using them in shell commands, for example
  `wslpath -u "$win_path"`.
- If a required `.exe` tool is unavailable, stop and report it instead of silently falling back
  to the WSL/Linux tool.

## Build, Test, and Development Commands

Slang build setup is platform-specific, especially under WSL. For compiler builds, use the
`slang-build` skill from `skills/slang-build` in the `shader-slang/slang-skills` repository
instead of following hard-coded commands in this file.

If the skill is unavailable because skills cannot be installed or network access is limited, use
`docs/building.md` as the fallback build reference.

Examples:

- `/slang-build build debug`: build the Debug configuration.
- `/slang-build rebuild debug`: discard the existing build directory and rebuild Debug.
- `/slang-build configure releasewithdebug`: configure an optimized build with symbols.
- `/slang-build clean`: rename and remove the existing build directory.

Do not infer WSL build commands from generic Linux instructions. Follow the platform detection,
host-tool selection, CMake preset choice, and clean-build steps defined by the skill.

After building, run tests from the repository root using the generated `slang-test` binary in
the directory for the selected configuration:

- `build/Debug/bin/slang-test`: run the Debug test suite.
- `build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 8`: run optimized
  tests with symbols in parallel using test servers.
- `build/Release/bin/slang-test -use-test-server -server-count 8`: run Release tests in
  parallel using test servers.

On Windows-hosted builds, use the `.exe` suffix if that is the generated binary name.

## Include Path Conventions

Prefer direct paths over relative traversal in `#include` directives. The `source/` directory is
on the compiler include path (exposed by the `core` CMake target), so cross-module headers are
reachable without `../`:

```cpp
// Preferred in new code
#include "core/slang-string.h"
#include "compiler-core/slang-source-loc.h"

// Existing code still uses the relative form; do not change it purely for style
#include "../core/slang-string.h"
#include "../compiler-core/slang-source-loc.h"
```

New files should use direct paths. Existing files need not be converted purely for style, but may
be opportunistically updated when the file is already being substantially modified for other
reasons (e.g., a security fix or feature addition touching many lines).

## Coding Style & Naming Conventions

Formatting:

- Use four-space indentation for C, C++, headers, and Slang files.
- Run `./extras/formatting.sh` before committing to apply rules from `.clang-format`
  and `.editorconfig`.
- Follow Allman braces, a 100-column limit, left-aligned pointers, and final newlines.

Conventions:

- Follow `docs/design/coding-conventions.md`.
- Avoid STL containers, iostreams, RTTI, and exceptions for ordinary errors.
- Use `UpperCamelCase` for types and `lowerCamelCase` for values.
- Use `SLANG_`-prefixed `SCREAMING_SNAKE_CASE` for macros.
- Prefer comments that explain why code exists.

Review conventions (recurring review feedback — following them avoids review round-trips):

- Comment functions in complete sentences: what it does first, then why if non-obvious; include a
  concrete example for non-trivial logic.
- Write explanatory comments in a conversational style. Prefer "Consider this example:" followed
  by the relevant user code over abstract labels such as "Full source shape", "AST trace", or
  "IR trace". After the example, explain what happens step by step in natural prose: which
  producer creates the AST/IR/value shape, what invariant this code is preserving, and which
  downstream consumer relies on it. Include enough of the original user code for the example to be
  understood without reconstructing the surrounding program from memory.
- Reuse before you write: check shared headers (`slang-ast-type.h`, `slang-ir-util.h`, the `*-util.h`
  files) for an existing helper (e.g. `isDeclRefTypeOf<T>`) before adding one. When the logic is
  genuinely new, extract it into a named, documented helper rather than an inline lambda/long block.
- Keep one source of truth for a mapping or classification, and delete any branch/fallback a refactor
  makes unreachable.
- Don't create a second AST/IR/`Val` representation of a value that already has one (it breaks
  `equals`/dedup); `SLANG_ASSERT` such invariants at the construction site.
- `SLANG_RELEASE_ASSERT` on out-of-contract input instead of silently returning a default.

## Shell Scripts

Scripts under `extras/` (and other repository shell scripts) must run on bash 3.2, the version
Apple ships as `/bin/bash` on macOS. Avoid bash 4+ only features such as `${var,,}`/`${var^^}`
case conversion, associative arrays (`declare -A`), `mapfile`/`readarray`, and namerefs
(`local -n`). Prefer portable equivalents (for example, lowercase with
`tr '[:upper:]' '[:lower:]'`). Validate with `bash -n script.sh` under the system bash.

## Testing Guidelines

Add tests near related coverage in `tests/`.

Slang tests:

- Use leading directives such as `//TEST(smoke):SIMPLE:`.
- Use `//DISABLE_TEST` only with a clear reason.
- For targeted runs, pass a prefix, for example
  `build/Debug/bin/slang-test tests/diagnostics/my-test`.

Unit tests live under `tools/slang-unit-test` and typically use `SLANG_UNIT_TEST(name)`.

## Problem-Solving Methodology

Follow the principled path, not the minimal-edit-distance path.

- Fix root causes, not symptoms. A bug surfacing in emit/codegen is usually caused upstream (an IR
  pass, lowering, type legalization, specialization, or the AST/IR representation). Trace it there.
- Question every change. If you cannot name a test that fails without a change, it probably should
  not exist. Ask whether the problem is telling you the direction/representation is flawed.
- Do not mask. A guard, null-check, or special case that papers over malformed AST/IR/witness-table
  data is a band-aid hiding a representation bug. Make the representation correct so consumers stay
  simple.
- Interrogate the input shape. For any code that handles a particular shape of input (AST node, IR
  inst, witness, type, ...), always ask: is that shape itself correct and principled, or should the
  upstream producer be fixed instead? Fix the producer when the shape is wrong; handle it here only
  when the shape is genuinely valid input. Record the answer in the PR description (Process report).
- Address conceptually unordered key→value data (witness-table / interface requirement entries) by
  role/key, never by position/index.
- Keep a working log throughout the task: the problem and a motivating example, how issues cascade
  (one fix exposing the next), the fix chosen for each and why it is principled (with a code trace),
  and rejected alternatives. Distill this log into the PR description; do not commit it.

### Self-Review for Unprincipled Changes

Before finalizing a non-trivial compiler change, review the diff for signs that the fix is
compensating for a bad AST/IR/`Val`/witness representation. Treat the following patterns as
high-risk until you can prove they are the right layer:

- A new custom equivalence relation over `DeclRef`, `Val`, `Type`, `Witness`, or IR shapes, such as
  recursive helpers named like `are...Equivalent`, `does...Match`, or `try...Match`. First ask why
  normal `substitute`, `resolve`, `getCanonicalType`, `equals`, or an existing canonical builder
  does not already make the two values identical.
- A new helper, fallback, or "try..." function that exists only to make one failing test pass. Audit
  every new helper, even small ones: if it redoes substitution, resolution, AST copy, generic
  solving, lookup, or lowering, it is probably hiding the actual invariant break.
- Code that converts checked semantic data back into syntax, such as rebuilding an `Expr` or
  `TypeExp` from a `Val`, `Type`, `DeclRef`, or witness. The checked semantic field should usually
  remain the source of truth; reconstructing syntax is a strong signal that a producer or copier is
  storing the wrong representation.
- Code that walks arbitrary operand graphs, substitution chains, witness chains, or lookup paths to
  rediscover context such as generic arguments, requirement keys, canonical paths, or parent
  declarations. The producer should usually store or construct the canonical form directly.
- Lowering, emit, specialization, or typeflow logic that patches a malformed AST/IR shape from an
  earlier phase. These consumers should be simple; if they need target-specific knowledge of a
  front-end representation accident, trace the producer instead.
- Hardcoded knowledge of particular `DeclRef` subclasses, builtin magic type names, generic
  argument indices, witness-table entry order, or nested-vs-flat specialization shape. Such code
  needs a strong invariant and should usually live at a canonical construction boundary.
- Guards that silently return a default value for an "impossible" shape. Use an assertion when the
  shape is truly out of contract; otherwise explain why the shape is valid input and add coverage.

Start each review by making a short inventory of every new helper/fallback/special case in the
diff. For each entry, record whether it survives, is reverted, or needs a producer-side fix. For
every flagged change, write down the input-shape audit before keeping it:

1. What exact shape reaches this code? Include a concrete example and the producing function.
2. Is that shape canonical and intentionally allowed, or is it an accidental alternative spelling?
3. If it is accidental, can the producer be fixed so downstream code uses the existing
   `substitute`/`resolve`/canonicalization path?
4. What semantic source of truth already exists, and is this code rebuilding syntax or structural
   shape from it instead of preserving it?
5. Which test fails if this change is removed, and does that test prove this layer is responsible?
   Do the revert drill when practical: remove the helper/special case, run the smallest failing
   test, and use the failure to identify the real producer-consumer break.
6. Can the special case be replaced by an assertion plus a producer-side fix, or by reusing an
   existing helper?

Do not keep a flagged change merely because it makes tests pass. If it remains necessary, the
`Process report` section of the PR description must justify why this input shape is valid and why
this layer owns the logic, with a code trace from producer to consumer.

## Commit & Pull Request Guidelines

- Use short, imperative commit subjects, for example `Reject invalid descriptor heap access`.
- Keep PRs small and based on `master`.
- PRs require passing workflows, review approval, and a `pr: non-breaking` or
  `pr: breaking change` label.
- Human contributors should sign the CLA when prompted.
- For formatting failures, run installed hooks from `./extras/install-git-hooks.sh` or
  request the format bot with `/format`.

Write the PR description in this five-part format:

1. **Motivation** — the problem, with a concrete example / motivating test case.
2. **Proposed solution** — the approach and why it is principled.
3. **Change summary** — the files/areas touched and what each does.
4. **Concepts and vocabulary** — a short glossary between the change summary and the process report.
   Restate only the codebase-specific or subtle terms the report relies on (e.g. witness, facet,
   the fixpoint solver, a non-obvious distinction the fix hinges on), as a reminder. Do not explain
   basic, well-known concepts (interface, associated type) — assume them.
5. **Process report** — explain every change with a logical reason. For a change addressing a
   cascading issue, describe the issue (with its motivating test case) and justify the fix with a
   code trace (the exact functions/insts involved), explaining why it is necessary and principled
   rather than a workaround. For any change that handles, guards, or special-cases a particular
   input shape, the report must answer the input-shape check from the methodology — is that shape
   correct and principled, or should its producer have been fixed instead? — so a reviewer can
   confirm the fix sits at the right layer.

Write for a reviewer without the full context in their head. Use the same conversational style
expected in code comments: start from a concrete user-code example, include the full relevant
snippet rather than just a type or function name, and explain the logical steps in order. Say what
the compiler builds, how that representation flows through named functions or IR instructions, and
why the chosen fix preserves the invariant. Avoid terse headings like "AST trace"; make the prose
read like an explanation to a reviewer who is learning the scenario for the first time.
