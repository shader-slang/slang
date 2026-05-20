# Prompt: tests-agentic/pipeline/01-lex-preprocess/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/pipeline/01-lex-preprocess/`,
anchored to
[`docs/llm-generated/pipeline/01-lex-preprocess.md`](../../../docs/llm-generated/pipeline/01-lex-preprocess.md).

Audience: nightly CI. The bundle exercises the first compilation
stage end-to-end — how the source buffer becomes a flat
post-preprocess token list — _without_ duplicating the pure-lexer
surface that `syntax-reference/tokens` already covers. The
preprocessor's directive set, macro expansion machinery, include
resolution, and the source-location preservation contract that lets
diagnostics walk back through expansion chains are the heart of this
bundle.

## Required structure

1. `BUNDLE.md` with the structure named in `_common.md`.
2. One test file per **verifiable claim** in
   `pipeline/01-lex-preprocess.md`. A claim is "verifiable" via
   `slangc` / `slangi` if its consequences can be observed in
   compilation behavior. The dominant claim families for this bundle
   are:
   - **lexer-side claims** the source doc adds beyond what
     `syntax-reference/tokens.md` already covers — chiefly
     backslash line continuation folding (with source-location
     preservation) and `<...>`-after-`#include` lexing.
   - **macro expansion claims** — object-like and function-style
     `#define`, argument substitution, `__LINE__` / `__FILE__` as
     constructed tokens, `#` stringization, `##` token paste.
   - **conditional compilation claims** — `#if` / `#ifdef` /
     `#ifndef` / `#else` / `#elif` / `#endif` flow; the
     "inactive branches are not expanded" rule; the "only
     `#if`-family directives are evaluated inside an inactive
     block" rule.
   - **`#include` claims** — that `#include` pushes a fresh stream
     for the resolved file and that included declarations are
     visible to the parent unit.
   - **failure-mode claims** — diagnostics for unbalanced `#if`,
     unknown directive, missing include, and the `#error`
     directive.

3. Each test is small, single-purpose, and named after the claim it
   verifies. Examples:
   - `object-macro-expansion.slang`
   - `function-macro-argument-substitution.slang`
   - `inactive-if-branch-not-expanded.slang`
   - `line-macro-is-constructed-token.slang`
   - `include-pushes-fresh-stream.slang`
   - `unbalanced-if-diagnostic.slang`

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/pipeline/01-lex-preprocess.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/llm-generated/syntax-reference/tokens.md`
- `docs/llm-generated/syntax-reference/keywords-and-builtins.md`
- `docs/llm-generated/pipeline/overview.md`

If you would cite anything else, stop and record a doc-gap finding
in `BUNDLE.md`.

## Source files you may consult for _verification only_

You may look at these files to verify that a claim in the doc is
realizable (e.g. the exact spelling of a directive, the diagnostic
code emitted for an unbalanced `#if`). You may **not** mine them
for behavioral claims that the doc does not make.

- `source/compiler-core/slang-lexer.h`
- `source/compiler-core/slang-lexer.cpp`
- `source/compiler-core/slang-token.h`
- `source/compiler-core/slang-token.cpp`
- `source/compiler-core/slang-token-defs.h`
- `source/slang/slang-preprocessor.h`
- `source/slang/slang-preprocessor.cpp`

If you find yourself thinking "this would cover the branch at
`slang-preprocessor.cpp:NNNN`", stop and re-read the doc.

## Test directives

Lex- and preprocessor-stage claims are target-independent: the
token list produced by phase 1 is the same regardless of which
backend later consumes it. So:

- Prefer `//TEST:INTERPRET(filecheck=CHECK):` for behavior that
  prints an observable value (numeric, char).
- For string-typed observations (`__FILE__`, stringization), use
  `//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute`
  and FileCheck the emitted C++ source — `slangi` `printf` does
  not support `%s`.
- For "is rejected" claims use
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`.
- For `#include`-of-a-real-file claims, the test file must declare
  `-I` to the bundle directory so the helper header resolves. The
  helper header lives next to the `.slang` file and is named
  `*.slang.h` (matching the convention used by `tests/preprocessor`).

Do not use any GPU-only directive. Do not add multi-target
duplicates for target-independent claims — that would add no
coverage and inflate runtime.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `pipeline/01-lex-preprocess.md` (or one of the listed
      secondary docs).
- [ ] At least one diagnostic test exists. The
      `#failure-modes` section makes explicit "is rejected"
      claims; a bundle with only positive tests is incomplete.
- [ ] Tests in this bundle do not duplicate the
      `syntax-reference/tokens` bundle. Pure lex-surface claims
      (numeric suffixes, comment shapes, raw strings) belong
      there. This bundle's lexer tests cover the doc claims
      _beyond_ those, e.g. line continuation and the
      `#include`-mode `<...>` token.
- [ ] No test depends on a GPU. `INTERPRET`, `-target cpp` text
      emit, and diagnostic-only directives are the only ones used.
- [ ] No test was written by inspecting an uncovered source line.
- [ ] `BUNDLE.md` `## Doc gaps observed` is honest. If you wanted
      a test you could not anchor — for example a token-paste
      semantics claim more specific than what the doc states —
      write down which claim the doc would need to add.
