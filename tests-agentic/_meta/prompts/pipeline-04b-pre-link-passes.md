# Prompt: tests-agentic/pipeline/04b-pre-link-passes/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/pipeline/04b-pre-link-passes/`,
anchored to
[`docs/llm-generated/pipeline/04b-pre-link-passes.md`](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md).

Audience: nightly CI. The bundle exercises the **pre-link IR-pass
sequence** that runs inside `generateIRForTranslationUnit` in
[slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp)
**before** the per-translation-unit IR module is cached and pulled
into `linkAndOptimizeIR` by `linkIR`. The doc organises the sequence
into four phases:

- **Phase A** — AST walk and IR emission (creates `IRModule`,
  emits debug source/compilation-unit insts, lowers entry points,
  walks every module-decl member, attaches NVAPI/Experimental
  decorations, validates).
- **Phase B** — Mandatory pre-optimisation transformations
  (`prelinkIR` to pull in `[unsafeForceInlineEarly]` bodies,
  `lowerErrorHandling`, `lowerDefer`, `synthesizeBitFieldAccessors`,
  `lowerExpandType`, optional `insertDebugValueStore`).
- **Phase C** — Mandatory optimisation passes (`constructSSA`,
  `applySparseConditionalConstantPropagation`, optional
  `simplifyCFG` + `peepholeOptimize`, per-function
  `eliminateDeadCode`, optional `invertLoops`,
  `performMandatoryEarlyInlining` fixed-point loop).
- **Phase D** — Non-essential validation, stripping, finalisation
  (`checkForRecursiveTypes`, `propagateConstExpr`,
  `checkForUsingUninitializedValues`, `checkForMissingReturns`,
  `checkAutoDiffUsages`, `checkForOperatorShiftOverflow`,
  `addDecorationsForGenericsSpecializedWithExistentials`,
  `checkForMeshOutputReads`, `stripFrontEndOnlyInstructions`,
  `stripImportedWitnessTable`, final `eliminateDeadCode`, optional
  `obfuscateModuleLocs`, structural validation,
  `buildMangledNameToGlobalInstMap`).

This is the pre-link, target-agnostic region; the post-link, per-
target sequence lives in `pipeline/05-ir-passes` and the
`target-pipelines/` bundles. Do not duplicate post-link claims here.

## The translation rule: claims to observations

The pre-link pipeline ends just before `module->buildMangledNameToGlobalInstMap`.
The `-dump-ir` flag emits a `### LOWER-TO-IR:` block whose body is
the IR snapshot at the end of Phase D (after all pre-link passes have
run, and before any post-link pass runs). That block is the primary
observation surface for this bundle.

Testable consequences:

- **"Pass X runs in the pre-link region"** — compile with
  `//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir
  -o /dev/null -entry main -stage compute` and FileCheck the
  `### LOWER-TO-IR:` block for the post-pass shape. The block header
  itself is constant text and a useful anchor:
  ```
  // CHECK-LABEL: ### LOWER-TO-IR:
  // CHECK-LABEL: func %main
  // CHECK: <opcode-X-or-its-replacement>
  ```
- **"Validation pass X emits diagnostic D"** — use
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` and match the diagnostic
  verbatim. Anchor at the same source line as the offending construct
  (per `_common.md` lessons on diagnostic placement).
- **"Pass X removes opcode Y"** — write source that would produce Y
  during Phase A lowering, then assert Y is absent from
  `### LOWER-TO-IR:` (e.g. `CHECK-NOT: defer`,
  `CHECK-NOT: tryCall`, `CHECK-NOT: ExpandType` if present).

### Observable claims (write tests for these)

#### Phase A: AST walk and IR emission

- **Entry point lowered as `IRFunc` with `[entryPoint(...)]` and
  `[numThreads(...)]` decorations** — observable in the
  `### LOWER-TO-IR:` block.
- **Global hashed string literal aggregate** — if `printf("...")`
  appears in the module, `### LOWER-TO-IR:` shows a
  `kIROp_GlobalHashedStringLiterals` aggregate. (May be absent if
  optimisation removed the printf; check via a side-effecting use.)

#### Phase B: mandatory pre-optimisation transformations

- **`lowerErrorHandling`** rewrites `throws` / `tryCall` into
  `Result<T,E>` + ordinary call + `ifElse`. After Phase B no
  `tryCall` survives.
- **`lowerDefer`** emits the `defer` body at each scope exit and
  removes the `defer` opcode. After Phase B no `defer` opcode survives
  and the deferred body appears inline.
- **`synthesizeBitFieldAccessors`** synthesises get/set helpers for
  bit-field declarations. Observable via the lowered backing field
  (`xx24bit_field_backing_*` name appearing in the emit text).
- **`lowerExpandType`** rewrites `IRExpandType` so the pattern is
  nested inside `IRExpand`. Observable when the input uses a
  variadic generic — the rewritten form appears in the
  `### LOWER-TO-IR:` block. If a clean Slang surface for variadic
  expand-type isn't available, log a doc gap.

#### Phase C: mandatory optimisation passes

- **`constructSSA`** promotes plain locals to SSA temporaries. A
  trivial `int x = a; x = x + 1; buf[0] = x;` shows up without
  `var`/`store`/`load` over a plain `Int` local in
  `### LOWER-TO-IR:`.
- **`applySparseConditionalConstantPropagation`** folds constant
  arithmetic. `int y = 7 + 3;` shows `store(..., 10 : Int)` rather
  than two literals plus an `add`.
- **`eliminateDeadCode`** (Phase C5 + D12) removes a top-level
  helper function that is never called. The dropped helper does not
  appear as `func %helper` in `### LOWER-TO-IR:`.
- **`performMandatoryEarlyInlining` loop** — exercised whenever
  `[__unsafeForceInlineEarly]` is in user code. That attribute is
  internal-only; do not write a user-code test of it. Record the
  loop's existence as a doc-anchored claim via the `prelinkIR` pull-
  in semantics test (whose body is brought in by the prelink + early
  inline cooperation).

#### Phase D: validation / stripping

- **`checkForRecursiveTypes`** (D1) rejects a self-referential
  struct. Verify by `DIAGNOSTIC_TEST`.
- **`checkForMissingReturns`** (D5) warns when a non-void function
  lacks a return on every control-flow path. Verify by
  `DIAGNOSTIC_TEST`.
- **`stripFrontEndOnlyInstructions`** (D10) removes
  `IRHighLevelDeclDecoration` and similar front-end-only insts. A
  `CHECK-NOT: HighLevelDecl` (or another doc-named front-end-only
  decoration) at the end of `### LOWER-TO-IR:` confirms this.
- **`buildMangledNameToGlobalInstMap`** (D15) — verifiable
  indirectly: the absence of a crash on a multi-decl module is the
  only surface signal, so do **not** spend a test slot on it.

#### Conditional gates

- **Loop-inversion gate** (`-loop-inversion` flag, gates C6).
  Compile with and without and assert the loop shape difference if
  observable. Skip if no clean surface signal exists.
- **Minimum-optimisations gate** (`-minimum-optimization`, gates
  C3/C4 + the inner simplification cluster). Skip unless the doc
  names a concrete observable effect.

### Not testable through slangc (record under `## Untested claims`)

- The **exact ordering** of A1…A10, B1…B6, C1…C7, D1…D15. The
  `### LOWER-TO-IR:` dump shows only the post-Phase-D state, not
  intermediate stages.
- `prelinkIR`'s pull-in of the `externalSymbolsToPrelink` set — set
  is populated by upstream lowering and is opaque to user source.
- `validateIRModuleIfEnabled` (A10, D14) — no-op unless a CLI flag
  is set; the agentic bundle does not exercise it.
- `obfuscateModuleLocs` (D13) — requires `-obfuscate` + source-map
  side-channel; out of scope for this bundle.
- The fixed-point convergence of `performMandatoryEarlyInlining` —
  `[__unsafeForceInlineEarly]` is internal-only and not a user
  surface.
- `insertDebugValueStore` (B6) — requires `-g`; observable via
  SPIR-V debug instructions but adds backend coupling unrelated to
  the pre-link claim.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for unobservable
   items.
2. 12–18 `.slang` test files. Most are
   `//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir
   -o /dev/null -entry main -stage compute` (the prompt's required
   directive). Two or three may be `DIAGNOSTIC_TEST` for the
   validation passes that surface diagnostics.

The `size_cap_files` cap is 50; staying in the 12-18 range keeps
each test laser-focused on one pre-link consequence.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/pipeline/04b-pre-link-passes.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for _verification only_

- `source/slang/slang-lower-to-ir.cpp` (`generateIRForTranslationUnit`,
  for the actual phase boundaries)
- `source/slang/slang-ir-lower-defer.cpp`
- `source/slang/slang-ir-lower-error-handling.cpp`
- `source/slang/slang-ir-bit-field-accessors.cpp`
- `source/slang/slang-ir-link.cpp`

You may **not** mine these for behavioural claims that the doc
does not make.

## Test directive

Default required directive for IR observation tests:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
```

Anchor `CHECK` patterns at `### LOWER-TO-IR:`, then at
`func %main` or `func %<helper>` inside that block. The block ends
at the next `^### ` line, so `CHECK-NOT` patterns after the entry
function naturally bound at the next pass boundary.

For validation diagnostics, use:

```
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main
```

## Lessons captured for pre-link tests

These are in addition to the universal lessons in `_common.md`.

- The `### LOWER-TO-IR:` dump captures the **post-Phase-D** state.
  Pre-link passes' effects are all visible inside that single block;
  there is no per-pass `### BEFORE / ### AFTER` in this region.
- Trivial-local SSA observation requires non-constant operands; per
  `_common.md`, feed inputs via `uniform`. The SCCP test, by
  contrast, needs **literal** operands so the fold is observable.
- `defer { ... }` lowers to inline emission of the body at scope
  exit, then `unconditionalBranch` to a join block. Anchor the
  CHECK at the order of the explicit `buf[0]` write followed by
  the deferred `buf[1]` write.
- `[__unsafeForceInlineEarly]` is internal and must not be used in
  user-code tests. Test the pre-link region's force-inline behaviour
  by observing what `prelinkIR` makes available — but the only
  user-visible surface is whether the module compiles. Treat the
  loop itself as out-of-scope.
- The `checkForRecursiveTypes` diagnostic uses error code `E41001`
  ("type contains cyclic reference"); the `checkForMissingReturns`
  warning uses `E41010` ("non-void function does not return in all
  cases"). Both attach to the offending declaration line.
- `checkForOperatorShiftOverflow` (D7) does NOT diagnose
  `1 << 40` reliably in the default option set; the validation pass
  is gated on `shouldRunNonEssentialValidation`. Treat it as
  out-of-scope unless a clean trigger is found.
- `printf` in user code routes through a hashed-string literal
  table. A `printf("hello!\n")` inside `main` causes a
  `GlobalHashedStringLiterals` aggregate to appear in the
  `### LOWER-TO-IR:` block when the printf is preserved by DCE.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `pipeline/04b-pre-link-passes.md`.
- [ ] All IR-observation tests use the canonical
      `-target spirv-asm -dump-ir -o /dev/null -entry main
      -stage compute` directive.
- [ ] Diagnostic tests use `DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` and
      verbatim error text from the runner's suggested annotations.
- [ ] No test depends on a GPU.
- [ ] No test asserts on the order in which the phases or passes
      run.
- [ ] No test exercises a post-link pass.
- [ ] `## Doc gaps observed` records pre-link claims that the doc
      makes but were not testable here (e.g. ordering of
      `### LOWER-TO-IR:` intermediate steps, `[__unsafeForceInlineEarly]`
      surface).
- [ ] `## Untested claims` records claims that are
      genuinely unobservable through any allowed directive
      (obfuscation, debug-info insertion, internal-only attributes).
