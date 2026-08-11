---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:25:31Z
target_doc: target-pipelines/spirv.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 11
actions:
  fixed: 7
  rejected_bogus: 1
  rejected_out_of_scope: 1
  deferred: 2
  escalated_to_finding: 0
---

# Gap-intake report for target-pipelines/spirv.md

## Summary

One gap was escalated: `4d492e51b04e` reports that `-preserve-params`
does not emit an unreferenced `IRGlobalParam`, but every watched-path
site agrees with the document, so this is a suspected compiler defect
rather than a documentation defect. Of the remaining ten, seven were
fixed with source-confirmed edits (front matter actually emitted by
`emitFrontMatter`, the register-allocation phi shape, the
`SV_*` system-value resolution plus the 49999 diagnostic, the
`SV_VertexID` / `SV_InstanceID` rebasing, the conservative-depth
execution modes, the transposed matrix-layout decoration, and the
meaning of `(always)` on row 33 of Phase C), one was rejected as bogus
(the `NoContraction` bullet already names `-fp-mode precise` and
already states that integer / bitwise / comparison opcodes are
skipped), one was rejected as out of scope (`-fvk-<class>-shift`
auto-allocation belongs to `pipeline/04c-layout-ir.md`, which watches
`slang-parameter-binding.cpp`), and one was deferred because the
sources that would settle it lie outside this document's
`watched_paths`. Two gaps against the same anchor
(`#legalizeentrypointsforglsl-despite-the-name`) were resolved as one
consolidated edit. Three suggested additions were written down
narrower than proposed, because the source says less than the
hypothesis did.

## Escalated gaps

- **`4d492e51b04e`** — `-preserve-params` and unreferenced global
  parameters. The document says the option makes Phase D emit
  unreferenced `IRGlobalParam`s, and the watched source says the same
  thing in three independent places:
  `source/slang/slang-emit.cpp:1353-1354` sets
  `deadCodeEliminationOptions.keepGlobalParamsAlive` from the option
  and every `eliminateDeadCode` inside `linkAndOptimizeIR` uses either
  that record or `fastIRSimplificationOptions.deadCodeElimOptions`,
  which is built from the same option
  (`source/slang/slang-ir-ssa-simplification.cpp:30-31, 44-45`); and
  `source/slang/slang-emit-spirv.cpp:12212-12218` walks the global
  insts and calls `ensureInst` on *every* `IRGlobalParam` when
  `shouldPreserveParams` holds, with no reachability or layout test.
  Outside the watched set the same picture holds:
  `source/slang/slang-ir-link.cpp:2340-2370` clones every global param
  and adds a `KeepAliveDecoration` when the option is set, and
  `source/slang/slang-options.cpp:4845` makes the target option set
  inherit the linkage option the CLI writes at
  `slang-options.cpp:2818-2821`, so the emit-time read of
  `targetProgram->getOptionSet()` should see it. The reported
  behaviour — no `OpVariable` for an unreferenced
  `RWStructuredBuffer` global under `-preserve-params`, with or
  without an explicit `[[vk::binding(3, 0)]]` — contradicts all of
  that. No existing finding in
  `docs/generated/tests/_meta/findings/` covers it; the operator will
  need one opened before `mark-gap-intake`. The reporting bundle
  shipped no test for this row (it is recorded as "not reproducible
  from the CLI"), and the tree's build is Linux x86-64 against an
  arm64 host, so intake could not re-run the compiler to narrow it
  further.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 7f255374e795 | rejected-bogus | The bullet's own lead already reads "**`NoContraction` under `-fp-mode precise`.**" and the bullet already says integer, bitwise, logical and floating-point-comparison operations "emit opcodes on which `NoContraction` is invalid" — both halves of the suggested addition are present. `source/slang/slang-emit-spirv.cpp:10415-10445` confirms the text: the comment names `-fp-mode precise` and the `switch` decorates only `OpFAdd` / `OpFSub` / `OpFMul` / `OpFDiv` / `OpFRem` / `OpFNegate` / `OpVectorTimesScalar`. | — |
| 46e0da77c461 | fixed | `source/slang/slang-emit.cpp:2573-2579` sets `useRegisterAllocation = true` for `isKhronosTarget && emitSpirvDirectly`; the emitted shape is pinned by `docs/generated/tests/design/target-pipelines/spirv/eliminate-phis-register-allocation.slang` (`%x = OpVariable %_ptr_Function_int Function` plus one `OpStore` per branch). The proposed "`OpLoad` at the merge" clause was dropped — no watched path or CHECK line pins it. | named the `Function`-storage `OpVariable` + per-edge `OpStore` shape that replaces the merge `OpPhi`, under `eliminatePhis with SPIR-V-specific options` |
| 3beeb23ba3da | fixed | `source/slang/slang-ir-glsl-legalize.cpp:51` (`getGLSLSystemValueInfo`), with `sv_dispatchthreadid` → `gl_GlobalInvocationID` at line 614 and `sv_isfrontface` → `gl_FrontFacing` at line 687; the unrecognized-semantic fall-through raises `Diagnostics::UnknownSystemValueSemantic` at line 1012, defined as error 49999 in `source/slang/slang-diagnostics.lua:4844-4849`. `docs/generated/tests/coverage/legalize/unknown-system-value-semantic.slang` pins it firing under `-target spirv`. | documented the `getGLSLSystemValueInfo` resolution step, two representative mappings, and the 49999 diagnostic; the full per-stage enumeration was deliberately not copied in — `source/slang/slang-emit-spirv.cpp:7663-7695` shows SPIR-V re-derives a `BuiltIn` decoration from the semantic name rather than using the `gl_*` name, so a `gl_*` table is catalog material for `pipeline/05-ir-passes.md`, not the observable form on this page (consolidated with 82b3ea5ed6b8) |
| 82b3ea5ed6b8 | fixed | `source/slang/slang-ir-glsl-legalize.cpp:4726` (`legalizeTargetBuiltinVar`) rewrites `HlslVertexID` loads to `SpvVertexIndex - SpvBaseVertex` (lines 4785-4814) and `HlslInstanceID` to `SpvInstanceIndex - SpvBaseInstance` (lines 4752-4783); it is called at line 5037, at the end of `legalizeEntryPointForGLSL`. `source/slang/slang-emit-spirv.cpp:7673-7680` maps those builtin names to `SpvBuiltInBaseVertex` / `SpvBuiltInBaseInstance` and requires `SpvCapabilityDrawParameters`. `docs/generated/tests/coverage/legalize/vertex-id-base-offset-glsl.slang` pins both subtractions. | added the `SV_VertexID` / `SV_InstanceID` rebasing note (with the `OpISub` + `DrawParameters` consequence and the un-rebased `SV_Vulkan*ID` contrast) as one consolidated edit with 3beeb23ba3da |
| fc1bc2f64511 | fixed | `source/slang/slang-emit-spirv.cpp:6066-6110` (`getDepthOutputExecutionMode`) maps `sv_depthgreaterequal` → `SpvExecutionModeDepthGreater` (line 6103) and `sv_depthlessequal` → `SpvExecutionModeDepthLess` (line 6109) off the var layout's `IRSystemValueSemanticAttr`, not off the decoration; `maybeEmitEntryPointDepthReplacingExecutionMode` (line 6138) requires `DepthReplacing` unconditionally at lines 6167-6170 and collapses a mixed entry point to it at lines 6154-6160. | named the emitted `DepthGreater` / `DepthLess` / `DepthReplacing` execution modes and the layout-attribute source the emitter derives them from |
| 4d492e51b04e | deferred | Downgraded from `escalated-to-finding` by the operator: a finding YAML requires `command` / `source_slang` / `observed_summary`, i.e. a reproduction, and none exists — the reporting bundle shipped no test ("not reproducible from the CLI") and this host cannot run `slangc` (Linux x86-64 build, arm64 host). The source trace below stands and should be re-escalated once someone can produce a repro. See `## Escalated gaps`. Watched source agrees with the document at `source/slang/slang-emit.cpp:1353-1354` and `source/slang/slang-emit-spirv.cpp:12212-12218`; the compiler does not. No finding id exists yet. | — |
| 80a7f7a3b324 | fixed | `source/slang/slang-emit-spirv.cpp:7045-7068` carries the inversion as a source comment ("the meaning of row/column major layout in our semantics is the *opposite* of what GLSL/SPIRV calls them") and emits `SpvDecorationRowMajor` for `SLANG_MATRIX_LAYOUT_COLUMN_MAJOR` and `SpvDecorationColMajor` otherwise. The accessor is `getMatrixLayoutMode()` at `source/slang/slang-compiler-options.h:316`. `docs/generated/tests/coverage/cli-options/matrix-layout-spirv-decoration.slang` CHECKs `ROW: OpMemberDecorate {{.*}} 0 ColMajor` and `COL: ... RowMajor`. | added a `getMatrixLayoutMode()` row to Option-set toggles stating that the emitted member decoration is the transpose of the source layout |
| a178c913175f | rejected-out-of-scope | The binding-shift behaviour is real but lives in `source/slang/slang-parameter-binding.cpp` (`_maybeApplyHLSLToVulkanShifts`, lines 4325-4378; the shift-enabled test at lines 1379-1393), which is a `watched_paths` entry of `pipeline/04c-layout-ir.md`, not of this document. This page's Option-set toggles table has no binding-shift row and makes no claim about shifts, so there is nothing here to correct. | — |
| f3b2b47f3963 | deferred | Both halves need sources outside this document's `watched_paths`: the shadowing front-end check (error 41001, `source/slang/slang-diagnostics.lua:4911`) is raised from `source/slang/slang-check-*.cpp`, and the surviving-input-shape question is answered by `source/slang/slang-ir-check-recursion.cpp`. Neither is watched here, and the claim cannot be settled by running the compiler on this host (Linux x86-64 build, arm64 host). Follow-up: either extend `watched_paths` with `slang-ir-check-recursion.cpp`, or re-file the note against `pipeline/05-ir-passes.md`, which owns the pass catalog. | — |
| 396a1bd6a8bf | fixed | `source/slang/slang-emit.cpp:2527` is an unconditional `SLANG_PASS(eliminateMultiLevelBreak, targetProgram)`, so `(always)` is a statement about the call site; row 34's `simplifyIR` (line 2530) and Phase D's simplification loop both run afterwards. The proposed "emits a `Function`-storage `bool` flag" wording was not written: `source/slang/slang-ir-eliminate-multilevel-break.cpp` is outside `watched_paths` and contains no boolean-variable construction, so attributing the `OpVariable %_ptr_Function_bool` seen in `multi-level-break-nested-loops.slang` to this pass would be an unconfirmed claim. | clarified on row 33 that `(always)` names the call site rather than a guaranteed IR change, generalized to every `(always)` row |
| 9ef7b8187353 | fixed | `source/slang/slang-emit-spirv.cpp:1657-1687` — `emitFrontMatter` emits only `OpCapability Shader` and `OpMemoryModel` (addressing model default `SpvAddressingModelLogical`, line 504). The rest of the proposed list is not front matter: `OpSource` comes from `emitSource` (line 2167, called at line 12246), and `SPV_KHR_storage_buffer_storage_class` is requested on demand at line 2527 for a `StorageBuffer`-storage-class pointer. The `; SPIR-V` / `; Version:` lines are the disassembler's rendering of the header words `emitPhysicalLayout` writes at lines 529-553. `docs/generated/tests/design/target-pipelines/spirv/memory-model-logical-glsl450.slang` pins `OpMemoryModel Logical GLSL450`. | listed what `emitFrontMatter` actually emits on Phase D row 15, and named where the other tokens in the suggested list really come from |
