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
- Address conceptually unordered key→value data (witness-table / interface requirement entries) by
  role/key, never by position/index.
- Keep a working log throughout the task: the problem and a motivating example, how issues cascade
  (one fix exposing the next), the fix chosen for each and why it is principled (with a code trace),
  and rejected alternatives. Distill this log into the PR description; do not commit it.

## Commit & Pull Request Guidelines

- Use short, imperative commit subjects, for example `Reject invalid descriptor heap access`.
- Keep PRs small and based on `master`.
- PRs require passing workflows, review approval, and a `pr: non-breaking` or
  `pr: breaking change` label.
- Human contributors should sign the CLA when prompted.
- For formatting failures, run installed hooks from `./extras/install-git-hooks.sh` or
  request the format bot with `/format`.

Write the PR description in this four-part format:

1. **Motivation** — the problem, with a concrete example / motivating test case.
2. **Proposed solution** — the approach and why it is principled.
3. **Change summary** — the files/areas touched and what each does.
4. **Process report** — explain every change with a logical reason. For a change addressing a
   cascading issue, describe the issue (with its motivating test case) and justify the fix with a
   code trace (the exact functions/insts involved), explaining why it is necessary and principled
   rather than a workaround.
