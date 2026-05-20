# Prompt: tests-agentic/glossary/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/glossary/`, anchored to
[`docs/llm-generated/glossary.md`](../../../docs/llm-generated/glossary.md).

Audience: nightly CI. The bundle exercises the **compiler-internals
vocabulary** defined in the glossary — but only those terms whose
definitions imply a behavior observable through `slangc` on a CI runner
without a GPU. Most glossary entries are conceptual (internal data
structures, builder helpers, file formats, IR opcode families) and have
no direct surface; those go into `## Out of scope` instead of becoming
tests.

## What is "testable" for this bundle?

A glossary entry is testable when the term's definition implies a
verifiable consequence reachable via:

- the slangc command line (e.g. `-target`, `-entry`, `-stage`,
  `-dump-ir`),
- the slangi byte-code interpreter (e.g. `static const` values,
  `printf` output, observed control flow),
- the diagnostic stream (an expected error code or wording),
- emitted target text for HLSL / GLSL / SPIR-V (asm) / Metal / WGSL /
  CUDA / CPP.

Examples of **testable** glossary entries:

- "entry point" — `-entry main -stage compute` selects a function;
  `[shader("compute")]` is recognized.
- "module" — a Slang file compiles to one module; `import` brings
  another module's symbols into scope.
- "translation unit" — passing multiple `.slang` files together vs.
  separately changes whether they share top-level scope.
- "diagnostic" — an ill-formed program produces an error with a
  specific code.
- "overload resolution" — between two candidates with different
  parameter types, the better-matching one wins.
- "type inference" — `var x = 42;` infers `int`.
- "shadowing" — an inner-scope name hides an outer-scope name with
  the same identifier.
- "inlining" / "dead-code elimination" / "monomorphization" /
  "specialization" — observable via `-dump-ir` patterns on uniform
  inputs.
- "prelude" — emitted target text includes prelude content for the
  selected backend.
- "target" / "target intrinsic" — `-target X` switches the emitted
  spelling of an intrinsic.

Examples of **not testable** (go to `## Out of scope`):

- "ASTBuilder", "IRBuilder", "FIDDLE", "scope" struct internals,
  "Linkage" object, "decl-ref", "lookup breadcrumb", "lookup mask",
  "lookup options", "lookup result", "DiagnosticSink" interface —
  internal data structures with no slangc surface.
- "fossil format", "RIFF container" — serialization back-ends not
  observable through normal `slangc` invocation.
- "control-flow graph", "dominator", "dataflow analysis", "SSA",
  "block parameter", "parent instruction", "hoistable instruction",
  "terminator instruction", "IRInst", "IROp", "IRFunc", "IRModule",
  "IRDecoration", "decoration" — IR-internal vocabulary; cannot be
  cleanly tested through `-dump-ir` because the dumps are not
  textually contract-stable for these aggregate concepts. (Specific
  decorations or opcodes belong to the `ir-reference/*` bundles.)
- "witness table", "existential type", "differential pair",
  "capability atom", "profile", "layout IR module", "mandatory
  optimization pass", "target legalization driver", "two-stage
  parsing", "core module", "partial generic application",
  "transparent member", "syntax-decl", "source-loc",
  "recursive descent" — either belong to a more specific peer
  bundle, or have no surface accessible from slangc text emission /
  diagnostics.
- "visibility" — has slangc surface (`public`/`internal`/`private`)
  but those keywords belong to the
  `name-resolution/visibility` bundle, not the glossary bundle.

The bar is **strict**: if you cannot write a test whose pass/fail is
directly tied to the glossary entry's one-paragraph definition, drop
the term after 3 attempts and record it under `## Out of scope`.

## Required structure

1. `BUNDLE.md` with the structure named in `_common.md`. The
   `## Doc gaps observed` section should be brief — the glossary is a
   lookup aid, not a contract surface; gaps mostly mean "term lacks an
   observable consequence" and should be listed under
   `## Out of scope` instead.
2. **10–18** `.slang` test files, one per testable glossary entry.
3. Each test file is small (≤ ~30 lines), single-purpose, and named
   after the term it verifies (kebab-case). Examples:
   - `module-import-brings-symbols.slang`
   - `entry-point-stage-flag.slang`
   - `overload-resolution-picks-best.slang`
   - `type-inference-var-from-int.slang`
   - `shadowing-inner-hides-outer.slang`
   - `prelude-emitted-for-cpp.slang`

## Doc anchors

The glossary has a single content section: `#terms`. Every glossary
entry's definition lives inside that one heading. All per-test
`doc_ref` values therefore use the same anchor:

```
docs/llm-generated/glossary.md#terms
```

And every per-test `doc_section_digest` is the digest of the `#terms`
section as a whole (compute once with
`python3 /tmp/section-digest.py docs/llm-generated/glossary.md terms`).
Use the **term name** in the `purpose` field to disambiguate which
entry is being verified.

## Test directives

Prefer the smallest directive that observes the claim:

- `//TEST:INTERPRET(filecheck=CHECK):` — for behaviors observable as
  a printed value or `static const` outcome (shadowing, overload
  resolution, type inference, module imports, inlining/DCE/spec
  outcomes whose computed value escapes via printf).
- `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` — for "produces error
  E####" or "rejected" claims (diagnostic, conversion cost cutoff).
- `//TEST:SIMPLE(filecheck=CHECK):-target hlsl|glsl|spirv-asm|metal|wgsl|cuda|cpp`
  — for emit-side claims (prelude, target intrinsic, target
  spelling), and for IR-observation claims that need `-dump-ir`.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` is
      `docs/llm-generated/glossary.md#terms`.
- [ ] The `purpose` line begins with the glossary term being verified
      (e.g. `purpose=module: import brings another module's ...`).
- [ ] No test exists for a conceptual / internal-only term. If you
      catch yourself writing one for `ASTBuilder` or `fossil format`,
      drop it and move the term to `## Out of scope`.
- [ ] No test depends on a GPU.
- [ ] Bundle has between 10 and 18 `.slang` files.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-check-overload.cpp:NNNN`", stop and re-read the glossary
      entry — and if the entry does not imply an observable, move the
      term out of scope.
