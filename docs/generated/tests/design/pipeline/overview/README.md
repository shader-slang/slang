---
generated: true
model: claude-opus-5[1m]
generated_at: 2026-08-04T00:00:00+00:00
source_commit: 7e725f15572c6589ee6d738a8856fb3348f11617
watched_paths_digest: 490dd1c9588fe1f513be42fbc0bf878dd784bfef04737d0e8c3366c97180a1dd
source_doc: docs/generated/design/pipeline/overview.md
source_doc_digest: 34d4cbcc71c2d9088ec50f94fd57774b0eece28660e4cb42f30dbba96a942be8
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for design/pipeline/overview

## Intent

This bundle exercises
[`docs/generated/design/pipeline/overview.md`](../../../../design/pipeline/overview.md),
the index to the per-stage pipeline documents. It is a **meta-bundle**:
most of the source doc is pointers into `01-lex-preprocess.md` …
`06-emit.md`, so single-stage claims (preprocessor diagnostics, parser
diagnostics, semantic-check diagnostics, the behaviour of an individual
IR pass, per-target emit quirks) are deliberately left to those bundles
and recorded under `## Doc gaps observed` as deferrals. What is tested
here is only what the overview asserts on its own account: that the
whole source-to-artefact data flow runs, that it runs once per target
request with a target-sensitive pass list in between, and that the four
decl kinds and the statement/expression shapes the AST-to-IR section
names actually reach the emitted text.

Coverage strategy: every claim that the doc states as target-observable
gets one file carrying a `//TEST:SIMPLE` directive per feasible
text-emit back-end — HLSL, GLSL, SPIR-V assembly, Metal, WGSL, CUDA and
C++ — with a distinct FileCheck prefix per target, because a
single-target test cannot show that a per-target dispatch or a
target-sensitive pass list exists at all. Boundary probes sit on the two
axes the doc's own claims imply: the minimum program (an empty entry
point) and control-flow nesting depth. One `INTERPRET` test covers the
non-source dispatch path, which has no target text to FileCheck.

## Functional coverage

| Claim                                                                                                                                                                                                                               | Intent     | Anchor                                                                        | Tests                                                                                      |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | ----------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------ |
| A function declaration in the AST becomes an IR function that reaches emit, appearing as a separate callee on the source back-ends and as an inlined body on SPIR-V.                                                                | functional | [#ast--ir-lowering](../../../../design/pipeline/overview.md#ast--ir-lowering) | [`ast-to-ir-lowers-function-decl.slang`](ast-to-ir-lowers-function-decl.slang)             |
| A for-loop statement in the AST becomes a back-edged block structure whose loop header, exit test, and body all reach the emitted target text.                                                                                      | functional | [#ast--ir-lowering](../../../../design/pipeline/overview.md#ast--ir-lowering) | [`ast-to-ir-lowers-loop.slang`](ast-to-ir-lowers-loop.slang)                               |
| A generic function declaration becomes an IR generic that the specialization passes turn into one concrete emitted function per instantiated type argument.                                                                         | functional | [#ast--ir-lowering](../../../../design/pipeline/overview.md#ast--ir-lowering) | [`ast-to-ir-lowers-generic.slang`](ast-to-ir-lowers-generic.slang)                         |
| A module-scope mutable variable becomes an IR global variable that reaches emit, legalized into whatever storage form each target provides.                                                                                         | functional | [#ast--ir-lowering](../../../../design/pipeline/overview.md#ast--ir-lowering) | [`ast-to-ir-lowers-global-var.slang`](ast-to-ir-lowers-global-var.slang)                   |
| A struct declaration in the AST becomes an IR struct type that survives the pass list and reappears as an aggregate type in the emitted target text.                                                                                | functional | [#ast--ir-lowering](../../../../design/pipeline/overview.md#ast--ir-lowering) | [`ast-to-ir-lowers-struct.slang`](ast-to-ir-lowers-struct.slang)                           |
| An if/else statement in the AST becomes basic blocks joined by a conditional branch, and that structure reaches the emitted target text.                                                                                            | functional | [#ast--ir-lowering](../../../../design/pipeline/overview.md#ast--ir-lowering) | [`ast-to-ir-lowers-control-flow.slang`](ast-to-ir-lowers-control-flow.slang)               |
| Expressions lower to SSA values and a value produced on both sides of a branch is carried to the join point by a block parameter, which reaches SPIR-V as an OpPhi.                                                                 | functional | [#ast--ir-lowering](../../../../design/pipeline/overview.md#ast--ir-lowering) | [`ssa-block-params-across-branch.slang`](ssa-block-params-across-branch.slang)             |
| Five levels of nested if/else lower to five nested conditional branches that all survive to the emitted target text.                                                                                                                | boundary   | [#ast--ir-lowering](../../../../design/pipeline/overview.md#ast--ir-lowering) | [`ast-to-ir-lowers-nested-control-flow.slang`](ast-to-ir-lowers-nested-control-flow.slang) |
| A compute entry-point declaration survives every stage and reaches emit as the selected back-end's own compute-kernel marker.                                                                                                       | functional | [#emit](../../../../design/pipeline/overview.md#emit)                         | [`entry-point-flows-to-target-marker.slang`](entry-point-flows-to-target-marker.slang)     |
| Emit selects a dispatch path per target request, so one source produces HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA, and C++ artefacts, each in that back-end's own idiom.                                                                | functional | [#emit](../../../../design/pipeline/overview.md#emit)                         | [`multi-target-emit-dispatch.slang`](multi-target-emit-dispatch.slang)                     |
| Non-source targets are dispatched away from the C-like source emitters, so the same front end plus IR passes also feed the VM bytecode path that the interpreter executes.                                                          | functional | [#emit](../../../../design/pipeline/overview.md#emit)                         | [`vm-bytecode-target-dispatch.slang`](vm-bytecode-target-dispatch.slang)                   |
| One non-trivial source traverses lex, preprocess, parse, check, lower, IR passes, and emit and emerges as a recognizable artefact on every text-emit target.                                                                        | functional | [#end-to-end-flow](../../../../design/pipeline/overview.md#end-to-end-flow)   | [`end-to-end-flow-all-targets.slang`](end-to-end-flow-all-targets.slang)                   |
| The smallest possible program -- a single entry point with an empty body and no parameters -- still traverses the whole pipeline and produces a target artefact.                                                                    | boundary   | [#end-to-end-flow](../../../../design/pipeline/overview.md#end-to-end-flow)   | [`end-to-end-flow-empty-entry-point.slang`](end-to-end-flow-empty-entry-point.slang)       |
| The IR transformation sequence between lowering and emit is target-sensitive, so one thread-group declaration is materialized in a different form on each target and dropped entirely on the targets that take it at dispatch time. | functional | [#ir-passes](../../../../design/pipeline/overview.md#ir-passes)               | [`pipeline-is-target-sensitive.slang`](pipeline-is-target-sensitive.slang)                 |
| Macro expansion and conditional compilation finish before parsing, so only the selected, fully expanded token text reaches the parser and every downstream stage.                                                                   | functional | [#lex--preprocess](../../../../design/pipeline/overview.md#lex--preprocess)   | [`preprocess-completes-before-parse.slang`](preprocess-completes-before-parse.slang)       |

## Untested claims

| Claim                                                                                                                                                                                                                                        | Reason                | Anchor                                                                                    | Why untested                                                                                                                                                                                                         |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------- | ----------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `#include` resolution completes before parsing begins, alongside lexing and macro expansion.                                                                                                                                                 | needs-multi-file-test | [#lex--preprocess](../../../../design/pipeline/overview.md#lex--preprocess)               | Observing include resolution needs a second file on the include path; a single `.slang` cannot express it. A two-file test whose included header declares the struct the entry point uses would verify it.           |
| Parsing is a two-stage process: at the decl stage a function body is captured as raw tokens, and at the body stage it is re-parsed lazily under the checker so generic-versus-comparison disambiguation has type information available.      | out-of-bundle         | [#parse-to-ast](../../../../design/pipeline/overview.md#parse-to-ast)                     | The observable consequence is which parse a `a < b > (c)` form receives, a parser-stage property. Covered by the `design/pipeline/02-parse-ast` bundle.                                                              |
| Semantic checking resolves names, attaches types, validates modifiers, performs overload resolution and conformance checking, and synthesizes default conformance witnesses and generated members.                                           | out-of-bundle         | [#semantic-check](../../../../design/pipeline/overview.md#semantic-check)                 | Each of these is a single-stage claim whose pass/fail depends only on the checker. Covered by the `design/pipeline/03-semantic-check` and `design/name-resolution/*` bundles.                                        |
| Roughly 160 `slang-ir-*.cpp` files implement the analyses, validations, specializations, legalizations, and target-specific lowerings that the pass list runs.                                                                               | internal-source-fact  | [#ir-passes](../../../../design/pipeline/overview.md#ir-passes)                           | A count of source files has no user-observable consequence. The testable part -- that the sequence is target-sensitive -- is covered by [`pipeline-is-target-sensitive.slang`](pipeline-is-target-sensitive.slang).  |
| Textual emit can also produce Torch glue code.                                                                                                                                                                                               | needs-cli-test        | [#emit](../../../../design/pipeline/overview.md#emit)                                     | `-target torch` emits nothing for an ordinary compute entry point; it needs a Torch-annotated entry point plus a host-side build to be meaningful. A wrapper script asserting non-empty glue output would verify it. |
| Downstream-compiled targets (DXIL, DXBC, metallib, PTX) and LLVM IR / native output via `slang-llvm` are dispatched separately from the source emitters.                                                                                     | gpu-dxc-dxil          | [#emit](../../../../design/pipeline/overview.md#emit)                                     | Each needs its downstream toolchain (dxc, fxc, the Apple metal compiler, nvrtc) present. CI nightly provisions them; the agent runner has none.                                                                      |
| `checkAllTranslationUnits` checks each translation unit once and registers the resulting module so later `import` decls resolve against it, then checks entry points before lowering.                                                        | needs-multi-file-test | [#driver-entry-points](../../../../design/pipeline/overview.md#driver-entry-points)       | The registration is only observable when a second translation unit imports the first. A two-file test importing a module declared in a sibling file would verify it.                                                 |
| `CompileRequestBase` holds the `Linkage` that owns the session and supplies the source manager, name pool, and file system, and `EndToEndCompileRequest` is what one `slangc` invocation becomes; `Module` is the front end's result object. | needs-unit-test       | [#driver-entry-points](../../../../design/pipeline/overview.md#driver-entry-points)       | These are C++ object-graph facts with no CLI surface. A unit test in `tools/slang-unit-test/` driving `ISession` / `IModule` would verify them.                                                                      |
| Diagnostics, the IR opcode catalog, targets and capabilities, the core module and preludes, and serialization are cross-cutting concerns that touch every stage.                                                                             | out-of-bundle         | [#cross-cutting-concerns](../../../../design/pipeline/overview.md#cross-cutting-concerns) | Each has its own bundle under `design/cross-cutting/`; duplicating them here would double-report the same failures.                                                                                                  |

## Doc gaps observed

| Anchor                                                                              | Kind                   | Gap                                                                                                                                                                                                                                                                                    | Suggested addition                                                                                                                                                                                                                     |
| ----------------------------------------------------------------------------------- | ---------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#ir-passes](../../../../design/pipeline/overview.md#ir-passes)                     | missing-example        | The section says the sequence is "target-sensitive" but gives no example of a source construct whose emitted shape differs per target, so a reader cannot tell whether "target-sensitive" means different passes run or only that emit spells one neutral shape differently.           | Add a two-line example showing one `[numthreads(16, 1, 1)]` entry point emitting an HLSL attribute, a GLSL `layout(local_size_x = 16 …)`, a SPIR-V `OpExecutionMode … LocalSize 16 1 1`, and nothing at all on Metal and CUDA.         |
| [#emit](../../../../design/pipeline/overview.md#emit)                               | ambiguous-claim        | The list "HLSL, GLSL, Metal, WGSL, C++, CUDA, or Torch glue" mixes six targets that emit for any entry point with Torch glue, which emits nothing unless the entry point is Torch-annotated. A reader following the list gets an empty artefact and no diagnostic.                     | Mark the Torch entry in the list as requiring a Torch-annotated entry point, and name the annotation, so a reader knows `-target torch` on an ordinary compute kernel is expected to produce nothing.                                  |
| [#emit](../../../../design/pipeline/overview.md#emit)                               | missing-surface        | The section names VM bytecode as a separately dispatched non-source target but never names the tool or invocation that requests it, so a reader cannot exercise that dispatch path from the doc alone.                                                                                 | Name `slangi` (and the `-target` spelling, where one exists) next to the "VM bytecode" mention, the way the textual targets are implicitly reachable through `slangc -target`.                                                         |
| [#ast--ir-lowering](../../../../design/pipeline/overview.md#ast--ir-lowering)       | undocumented-behavior  | The section says "block parameters carrying values across control-flow edges" without noting that the block-parameter form is only established by an optimization pass; at the default optimization level the value is round-tripped through a function-scope variable instead.        | Add a sentence stating at which point block parameters replace the initial variable round-trip, and note that the phi form is only visible in emitted SPIR-V when optimization is enabled.                                             |
| [#end-to-end-flow](../../../../design/pipeline/overview.md#end-to-end-flow)         | cascading-only-mention | The section notes that checking and parsing weave together via the two-stage parser but the only observable consequence, generic-versus-comparison disambiguation, is a parser-stage property. Deferred to `design/pipeline/02-parse-ast`; noted so the overview is not double-tested. | Add a "tested in" pointer next to the two-stage-parser remark so a reader (and the next test-generation pass) knows the claim's coverage lives in the parse-stage document.                                                            |
| [#driver-entry-points](../../../../design/pipeline/overview.md#driver-entry-points) | missing-surface        | The section is written entirely in terms of C++ types and line numbers (`FrontEndCompileRequest`, `checkAllTranslationUnits` "line 498 at `source_commit`"), with no user-visible behaviour attached, so no claim in it can be anchored by a compiler-output test.                     | For each named object add one sentence saying what a user of `slangc` or the public API observes because of it — for example that module registration is why a later `import` in a second file resolves without recompiling the first. |
| [#semantic-check](../../../../design/pipeline/overview.md#semantic-check)           | cascading-only-mention | The section enumerates the checker's responsibilities but every one of them is single-stage, so an overview-level test would only duplicate the stage bundle. Deferred to `design/pipeline/03-semantic-check` and `design/name-resolution/*`.                                          | Add a "tested in" pointer to the per-stage document next to the responsibility list, so the overview reads as an index rather than as an independent source of claims.                                                                 |
| [#lex--preprocess](../../../../design/pipeline/overview.md#lex--preprocess)         | missing-example        | The claim that "`#include` resolution all complete before parsing begins" has no example, and unlike macro expansion it cannot be demonstrated within one file, so a reader cannot check it without inventing a second file and an include path.                                       | Add a two-file example (a header declaring a type, a source file including it) and state what in the emitted artefact shows the include was resolved before parsing.                                                                   |

## Claims

Enumerated from the source doc, grouped by its headings. Every claim is
either covered in `## Functional coverage` or listed in
`## Untested claims`.

**`## End-to-end flow`**

1. Data flows source → lex/preprocess → parse → check → lower → IR
   passes → emit → target artefact.
2. The diagram's ordering is conceptual; parsing and checking are
   interleaved by the two-stage parser.
3. The IR pass list is target-sensitive.

**`### Lex / Preprocess`**

4. Source buffers are read and turned into a flat token array.
5. Lexing, preprocessing, and `#include` resolution all complete before
   parsing begins.

**`### Parse to AST`**

6. Recursive-descent parsing produces a strongly-typed AST.
7. At the decl-parsing stage a function body is captured as raw tokens;
   at the body-parsing stage it is re-parsed lazily under the semantic
   checker, so generic/comparison disambiguation has type information.

**`### Semantic check`**

8. Semantic checking resolves names, attaches types, validates
   modifiers, performs overload resolution and conformance checking, and
   synthesizes default conformance witnesses and generated members.

**`### AST → IR lowering`**

9. Decls become IR global variables, functions, struct types, and
   generics.
10. Control-flow statements create basic blocks and branches; ordinary
    statements emit instructions into the current block.
11. Expressions become SSA value instructions, with block parameters
    carrying values across control-flow edges.

**`### IR passes`**

12. A long, target-sensitive sequence of IR transformations runs between
    lowering and emit.
13. Roughly 160 `slang-ir-*.cpp` files implement those passes.

**`### Emit`**

14. Emit selects a dispatch path per target request.
15. Textual targets go to a C-like source emitter — HLSL, GLSL, Metal,
    WGSL, C++, CUDA, or Torch glue.
16. Binary and non-source targets (direct SPIR-V, downstream-compiled
    DXIL/DXBC/metallib/PTX, LLVM IR / native, and VM bytecode) are
    dispatched separately.

**`## Driver entry points`**

17. `CompileRequestBase` holds the `Linkage` that supplies the source
    manager, name pool, and file system used through parse and check.
18. `checkAllTranslationUnits` checks each unchecked translation unit
    once, registers the checked module so later `import` decls find it,
    and checks entry points before lowering.
19. `EndToEndCompileRequest` is what one `slangc` invocation becomes,
    and `Module` is the front end's result object (AST + IR).

**`## Cross-cutting concerns`**

20. Diagnostics, the IR opcode catalog, targets and capabilities, the
    core module and preludes, and serialization touch every stage and
    are documented outside the per-stage docs.
