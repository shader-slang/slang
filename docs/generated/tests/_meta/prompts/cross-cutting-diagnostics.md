# Prompt: docs/generated/tests/cross-cutting/diagnostics/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/cross-cutting/diagnostics/`,
anchored to
[`docs/generated/design/cross-cutting/diagnostics.md`](../../../design/cross-cutting/diagnostics.md).

Audience: nightly CI. The bundle exercises the **diagnostic surface**
of the compiler — which severity, which code, and which message text
the user sees when a given condition is detected — and the controls
that let users suppress, downgrade, or upgrade those diagnostics. It
does **not** test what each individual diagnostic-emitting code path
does internally; it tests only that the doc's claims about how the
diagnostic system behaves are observable through `slang-test`.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. One test file per **verifiable diagnostic-system claim** in the
   source doc. The doc is mostly meta — it describes how diagnostics
   are declared, what fields they have, and how they reach the user.
   The testable consequences observable to `slangc` callers are:

   - **severity-name claims** — the rendered names of `Severity::Error`,
     `Severity::Warning`, `Severity::Note` (`error`, `warning`, `note`)
     appear in formatted output for diagnostics emitted at those
     severities. Verify by triggering one diagnostic at each severity
     and FileChecking the severity word.
   - **error-code claims** — every diagnostic has a unique integer id
     rendered as `E#####` (or `W#####`-style for warnings — observed as
     the integer code with the severity prefix in `slangc` output).
     Verify by FileChecking the literal `E#####` code on the
     diagnostic line. Pick codes that are stable and central
     (`E20003` unexpected-token, `E30015` undefined-identifier,
     `E30201` function-redefinition, etc.) — use real codes from
     `source/slang/slang-diagnostics.lua` for verification only.
   - **source-location claims** — the diagnostic's caret aligns with
     the source column of the offending token. Verify with caret-style
     `//CHECK:` annotations that pin the carets to specific columns
     (this is what `DIAGNOSTIC_TEST` natively checks).
   - **note claims** — a diagnostic with a `note` (e.g.
     `function-redefinition` carries a "see previous definition" note)
     renders the note as a `Severity::Note` companion. Verify with a
     `//CHECK: note` annotation on the note's line.
   - **rich-diagnostic parameter interpolation** — the doc shows
     `{found}`/`{expected}` interpolation in rich diagnostics. The
     observable consequence is that the type names appear quoted in
     the rendered text. Verify by triggering a type-mismatch and
     FileChecking the actual quoted types.
   - **suppression controls** — `#pragma warning(disable: <id>)`
     suppresses a warning. Verify with an `intent=functional` test
     that emits a `//TEST:SIMPLE` directive with a `CHECK-NOT:`
     pattern to assert the warning code does **not** appear.

3. Each test is small, single-purpose, and named after the claim it
   verifies. Examples:
   - `severity-name-error.slang`
   - `severity-name-warning.slang`
   - `severity-name-note.slang`
   - `error-code-undefined-identifier.slang`
   - `caret-points-at-offending-token.slang`
   - `pragma-warning-disable-suppresses.slang`

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/cross-cutting/diagnostics.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly hands off):

- `docs/diagnostics.md`
- `docs/diagnostic-guidelines.md`
- `docs/generated/design/architecture/overview.md`

If you would cite anything else, stop and record a doc-gap finding in
`README.md`.

## Source files you may consult for _verification only_

You may look at these files to confirm an error code, severity, or
diagnostic name. You may **not** mine them for behaviors the doc does
not claim.

- `source/compiler-core/slang-diagnostic-sink.h` (`Severity` enum,
  `getSeverityName`)
- `source/compiler-core/slang-diagnostic-sink.cpp`
- `source/compiler-core/slang-core-diagnostics.h`,`.cpp`
- `source/slang/slang-diagnostics.h`,`.cpp`
- `source/slang/slang-diagnostics.lua` — the canonical list of
  diagnostic codes and message text. Use to pick a stable code that
  fires under a known input.
- `source/slang/slang-rich-diagnostics.h`,`.cpp`

## Test directives

Diagnostics are pre-codegen and target-independent for almost every
claim in this bundle. Pick one directive per file:

- `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` — for "this code triggers
  this diagnostic" claims. CHECK annotations match by message text,
  by severity word, or by error code (`E#####`). Caret position is
  enforced when the caret is included on the CHECK line.
- `//TEST:SIMPLE(filecheck=CHECK):-target hlsl` (or any text target)
  — only when you need `CHECK-NOT:` to assert that a warning was
  suppressed. The `DIAGNOSTIC_TEST` runner is exhaustive by default
  (every emitted diagnostic must be matched); a no-diagnostic test
  is cleaner expressed with a regular `//TEST:SIMPLE` directive
  and a `CHECK-NOT` over the absence of the code.

Do **not** add gratuitous multi-backend directives. Diagnostics fire
pre-codegen, so the same diagnostic surfaces from every target — a
single directive is enough. The exception is a target-specific
diagnostic (e.g. "this is rejected for target spirv but accepted for
hlsl"); the doc does not currently claim any of those, so this
exception probably does not apply here.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `cross-cutting/diagnostics.md` (or one of the listed secondary
      docs).
- [ ] Each diagnostic test uses a **real** diagnostic code drawn
      from `slang-diagnostics.lua` (verification use only).
- [ ] Severity tests cover at least `error`, `warning`, and `note`.
- [ ] At least one test exercises caret column alignment (the doc's
      claim that `SourceLoc` is rendered with caret rendering).
- [ ] At least one test exercises a multi-span diagnostic (primary
      + note), since the doc explicitly describes notes.
- [ ] At least one test exercises a suppression control
      (`#pragma warning(disable: <id>)` is the user-facing surface).
- [ ] No test depends on a GPU.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-diagnostic-sink.cpp:NNNN`", stop and re-read the doc.

## Lessons captured from the pilot bundle (syntax-reference/tokens)

These apply here too:

- `DIAGNOSTIC_TEST` matches caret columns precisely; use real
  alignment, not approximate. The runner prints "Suggested
  annotations you can copy" on mismatch — that is the source of
  truth for column numbers.
- The default mode is **exhaustive**: every emitted diagnostic must
  be matched. If you want to assert only a specific diagnostic and
  ignore cascading errors, use the `non-exhaustive` option:
  `//DIAGNOSTIC_TEST(non-exhaustive):SIMPLE(diag=CHECK):`. Prefer
  exhaustive when feasible — it catches drift in the diagnostic set
  for the same input.
- `slangi` is not the right runner for diagnostic tests — it suppresses
  some of the rich formatting that `DIAGNOSTIC_TEST` matches against.
  Use `DIAGNOSTIC_TEST` directly.
