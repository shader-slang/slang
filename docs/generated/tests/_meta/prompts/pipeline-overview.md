# Prompt: docs/generated/tests/pipeline/overview/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/pipeline/overview/`,
anchored to
[`docs/generated/design/pipeline/overview.md`](../../../design/pipeline/overview.md).

Audience: nightly CI. This is a **meta-bundle**. The source doc is the
index to the per-stage pipeline documents — most of its content is
pointers ("see `01-lex-preprocess.md`", "see `06-emit.md`"). The tests
here cover only the **end-to-end pipeline claims** that the overview
doc makes itself and that would not be more naturally tested in a
stage-specific bundle:

- **One source compiles to multiple targets.** The emit dispatcher
  selects the right backend per `TargetRequest` and produces a target
  artefact in HLSL, GLSL, SPIR-V, Metal, WGSL, C++, or CUDA text.
- **The pipeline is per-target.** The IR pass list is target-sensitive,
  so the same source emits target-shaped (not just textually identical)
  output for each backend.
- **The full data flow happens.** A non-trivial source successfully
  traverses lex → preprocess → parse → check → lower → IR passes →
  emit and emerges as recognizable target text.
- **AST → IR lowering integrates with IR passes and emit.** A struct
  or control-flow construct in source survives the round trip and
  appears in target text in a target-appropriate form.
- **Entry points survive the pipeline.** A `[shader("compute")]`
  attribute in the source produces an entry-point marker in the
  emitted target text (`[numthreads]`, `@compute`, `kernel`,
  `__global__`, …).

If a behavior is more naturally a stage-specific claim — for example
"the preprocessor emits a diagnostic for `#error`" or "the parser
reports E20001 on a syntax error" or "the checker reports an
undefined-identifier diagnostic" — it belongs in the per-stage bundle
(`01-lex-preprocess`, `02-parse-ast`, `03-semantic-check`). The
overview bundle records those as "deferred to <bundle>" in
`## Doc gaps observed`.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. One test file per **verifiable end-to-end claim** in
   `pipeline/overview.md`. Each test should not duplicate a stage-
   specific bundle. The dominant claim families are:
   - **multi-target dispatch** — single source emits to several
     backends.
   - **target-sensitive IR pass list** — the same source yields
     target-shaped output divergence (HLSL `[numthreads(...)]` vs
     GLSL `layout(local_size_x...)` vs SPIR-V `OpExecutionMode`).
   - **AST → IR → emit round trip** — a struct or `if/else` in source
     reaches the emitted target text.
   - **entry-point flow** — `[shader("compute")]` lands as the
     target's compute-kernel marker.

3. Each test is small, single-purpose, and named after the claim it
   verifies. Examples:
   - `multi-target-emit-dispatch.slang`
   - `pipeline-is-target-sensitive.slang`
   - `ast-to-ir-lowers-struct.slang`
   - `entry-point-flows-to-target-marker.slang`

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/pipeline/overview.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/generated/design/architecture/overview.md`
- `docs/generated/design/pipeline/01-lex-preprocess.md`
- `docs/generated/design/pipeline/02-parse-ast.md`
- `docs/generated/design/pipeline/03-semantic-check.md`
- `docs/generated/design/pipeline/04-ast-to-ir.md`
- `docs/generated/design/pipeline/05-ir-passes.md`
- `docs/generated/design/pipeline/06-emit.md`

Each per-stage doc is the canonical home of its stage's claims; this
bundle should only cite the overview, and reach into a per-stage doc
only for "the X stage is observable end-to-end" claims that are
fundamentally about the data flow, not about the stage's internals.

If you would cite anything else, stop and record a doc-gap finding in
`README.md`.

## Source files you may consult for _verification only_

You may look at these files to verify that a claim in the doc is
realizable (e.g. that the dispatcher actually has a path for each
backend, that target-specific IR-pass branches exist). You may
**not** mine them for behavioral claims that the doc does not make.

- `source/slang/slang-compile-request.cpp`, `.h`
- `source/slang/slang-emit.cpp`
- `source/slang/slang-lower-to-ir.cpp`
- `source/slang/slang-check.cpp`
- `source/slang/slang-parser.cpp`
- `source/slang/slang-preprocessor.cpp`
- `source/compiler-core/slang-lexer.cpp`

If you find yourself thinking "this would cover the branch at
`slang-emit.cpp:NNNN`", stop and re-read the doc.

## Test directives

End-to-end pipeline claims are by nature **target-sensitive**: the
emit dispatcher and the IR pass list both branch on target, so a
single-target test cannot prove the dispatcher exists or that
per-target divergence happens. The default for this bundle is:

- Carry **multiple `//TEST:SIMPLE` directives** in a single file,
  one per backend that is observable in the no-GPU runner: HLSL,
  GLSL, SPIR-V (asm), Metal, WGSL, CUDA, CPP. Use distinct
  `filecheck=<NAME>` labels and FileCheck patterns per target —
  e.g. `// HLSL: ...`, `// GLSL: ...`, `// SPIRV: ...`.
- Avoid square brackets in CHECK patterns when matching against
  Slang source that uses attribute brackets — FileCheck's `[[ ]]`
  is parsed as a variable. Match a bare token (`numthreads`,
  `kernel`) instead of the bracketed form.
- Use `INTERPRET` only for cross-cutting observations that have no
  per-target text shape; for an overview bundle, that is rarely
  the right choice.
- Diagnostic claims belong in stage-specific bundles. This bundle
  should not use `DIAGNOSTIC_TEST` unless the claim is genuinely
  cross-stage (and even then, prefer recording the gap).

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `pipeline/overview.md` (or one of the listed secondary docs,
      and only when the overview hands off).
- [ ] Tests are end-to-end. A test whose pass/fail depends on only
      one stage (lex, parse, check, lower, …) belongs in the
      stage-specific bundle and should be recorded in
      `## Doc gaps observed` as "deferred to <bundle>".
- [ ] At least one test exercises **all observable text backends**
      (HLSL, GLSL, SPIR-V asm, Metal, WGSL, CUDA, CPP). The whole
      point of an overview bundle is multi-target coverage.
- [ ] No test depends on a GPU. `SIMPLE` text-emit and (rarely)
      `INTERPRET` are the only directives used.
- [ ] No test was written by inspecting an uncovered source line.
      If you find yourself thinking "this would cover the branch
      at `slang-emit.cpp:NNNN`", stop and re-read the doc.
- [ ] `## Doc gaps observed` lists the stage-specific claims that
      were intentionally deferred (preprocessor diagnostics →
      `01-lex-preprocess`; parser diagnostics → `02-parse-ast`;
      semantic-check diagnostics → `03-semantic-check`; etc.).
