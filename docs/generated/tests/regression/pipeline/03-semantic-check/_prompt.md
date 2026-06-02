# Prompt: docs/generated/tests/regression/pipeline/03-semantic-check/

> This is a representative per-section prompt template. Phase B1 will
> add a sibling prompt for each bundle key in `manifest.yaml`. The
> shape — `## Target`, `## Required structure`, `## Quality checklist`
> — should be reused for those. See also `_common.md` for the rules
> that apply to every bundle.

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/regression/pipeline/03-semantic-check/`,
anchored to
[`docs/generated/design/pipeline/03-semantic-check.md`](../../../design/pipeline/03-semantic-check.md).

Audience: nightly CI. The bundle exercises semantic checking
end-to-end: declaration checking, expression checking, statement
checking, type checking, overload resolution, conformance, conversion,
inheritance, and modifier checking.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. A test file per **verifiable claim** in `pipeline/03-semantic-check.md`.
   A claim is "verifiable" if it makes a statement about observable
   behavior of `slangc`:
   - "X is accepted" — write a positive test (`//TEST:SIMPLE(filecheck=CHECK):`).
   - "X is rejected with diagnostic D" — write a diagnostic test
     (`//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`).
   - "X means semantically Y" — write a CPU compute test
     (`//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type`)
     or an interpreter test (`//TEST:INTERPRET(filecheck=CHECK):`)
     that observes Y.
   - "X produces IR I" — write an IR-dump test
     (`//TEST:SIMPLE(filecheck=CHECK): -target spirv -dump-ir -o /dev/null`).

3. Coverage per claim:
   - At least one positive test.
   - At least one negative / diagnostic test for any claim that
     mentions an error condition or an "is rejected" behavior.
   - Where the doc enumerates multiple targets for the same claim, a
     test per target if it is reasonable to do so without a GPU
     (SPIR-V text, HLSL text, GLSL text, MSL text, WGSL text, CUDA
     text, IR dump, `slangi`).

4. Naming: `<claim-shortname>-<axis>.slang`, e.g.
   `overload-prefer-specialized.slang`,
   `overload-prefer-specialized-negative.slang`,
   `type-conversion-int-to-float-cpu.slang`.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/pipeline/03-semantic-check.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly references them):

- `docs/generated/design/ast-reference/declarations.md`
- `docs/generated/design/ast-reference/expressions.md`
- `docs/generated/design/ast-reference/types.md`
- `docs/generated/design/name-resolution/lookup.md`
- `docs/generated/design/name-resolution/overload-resolution.md`
- `docs/generated/design/cross-cutting/diagnostics.md`
- `docs/user-guide/02-conventional-features.md`
- `docs/language-reference/expressions.md`
- `docs/language-reference/declarations.md`

If you would cite something else, stop and instead record a doc-gap
finding in `README.md`.

## Source files you may consult for _verification only_

You may look at these files to verify that a claim in the doc is
syntactically realizable (e.g. the spelling of a keyword, the exact
diagnostic message text). You may **not** mine them for behavioral
claims that the doc does not make.

- `source/slang/slang-check.cpp`
- `source/slang/slang-check-decl.cpp`
- `source/slang/slang-check-expr.cpp`
- `source/slang/slang-check-stmt.cpp`
- `source/slang/slang-check-type.cpp`
- `source/slang/slang-check-overload.cpp`
- `source/slang/slang-check-conversion.cpp`
- `source/slang/slang-check-inheritance.cpp`
- `source/slang/slang-check-modifier.cpp`
- `source/slang/slang-diagnostics.lua` (for diagnostic codes and
  message text)

## DIAGNOSTIC_TEST lessons (gotchas observed during this bundle)

- Caret columns are matched **exactly**. The `^` (or `^^^...`) on
  every `//CHECK:` / `/*CHECK:` line must align to the actual
  column range of the offending token in the rendered diagnostic.
  When unsure, run the test once and copy the runner's "Suggested
  annotations you can copy" block — it is authoritative.
- A `^` at source-column 1–9 is impossible to express with `//CHECK:`
  (the `//CHECK:` prefix consumes those columns). Use the
  `/*CHECK: ... */` block-comment form, where the carets sit on
  fresh lines under the source line they pin.
- `non-exhaustive` is **an argument inside the `(diag=CHECK, ...)`
  parens**, not a top-level prefix:
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):`.
- Exhaustive mode rejects a test that has `non-exhaustive` but
  annotates every emitted diagnostic ("Unnecessary 'non-exhaustive'").
  Use `non-exhaustive` only when there are companion diagnostics
  you intentionally do not want to pin (e.g. "candidate: …" notes
  attached to an overload-resolution failure).
- Some diagnostics attach to a different line than the user-visible
  offending one — e.g. `missing-return` (W41010) attaches to the
  function's signature line, not the closing brace. Place the
  `/*CHECK:` block immediately after that line.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an existing anchor in
      `pipeline/03-semantic-check.md` (or one of the allowed
      secondary docs listed above).
- [ ] At least one negative / diagnostic test exists. Semantic check
      is mostly about saying "no"; a bundle with only positive tests
      is incomplete.
- [ ] Diagnostic tests cite a real diagnostic code (look it up in
      `source/slang/slang-diagnostics.lua` — verification use only).
- [ ] No test depends on a GPU. CPU compute, `slangi`, text-emit, and
      diagnostic-only directives are the only ones used.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-check-overload.cpp:1234`", stop. Re-read the doc and only
      add the test if the doc supports it.
- [ ] README.md `## Doc gaps observed` is honest. If you wanted a
      test you could not anchor, write down which claim the doc would
      need to add.
