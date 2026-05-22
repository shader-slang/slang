---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:44:47Z
source_commit: 2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_doc: docs/llm-generated/ir-reference/misc.md
source_doc_digest: d3e825874b20af6a9aee9ec05258024460a36074969159f4b86b451f74857f87
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/misc

## Intent

Tests verify the catch-all IR opcodes documented in
[`docs/llm-generated/ir-reference/misc.md`](../../../docs/llm-generated/ir-reference/misc.md)
that have an observable LOWER-TO-IR surface in portable Slang
shader code: the compile-time queries `sizeOf` / `alignOf` /
`countOf` (Size, alignment, count), the run-time type predicate
`IsType` (Type queries and predicates), the variadic-generic
projection introducer `Expand` (Pack and expansion), and the
literal-hash opcode `getStringHash` (String hashing).

The observation mechanism is uniformly `-target spirv-asm -dump-ir
-o /dev/null -stage compute -entry main`, with FileCheck patterns
anchored at user-named helper functions inside the IR dump so the
large preamble does not contribute false positives. Each query is
wrapped in a generic where appropriate so that constant-folding at
LOWER-TO-IR does not collapse the opcode to its folded value.

The remaining opcodes in `misc.md` (system placeholders, tensor /
runtime helpers, the remaining Pack opcodes beyond `Expand`, the
synthesized type predicates like `IsInt` / `IsBool`, storage-type
legalization casts, annotations, liveness markers, and kernel-
launch ops) are all marked `(synthesized)` in the source doc and
have no portable shader-language surface that reliably produces
them on the LOWER-TO-IR stage. They are recorded under
`## Untested claims` below.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| The `expand` keyword on a variadic value pack lowers to the `Expand` IR opcode that introduces an iteration scope over the pack. | functional | [#pack-and-expansion](../../../docs/llm-generated/ir-reference/misc.md#pack-and-expansion) | [`expand-variadic-pack.slang`](expand-variadic-pack.slang) |
| Both `sizeOf` and `alignOf` opcodes carry an optional data-layout operand (the doc lists `dataLayout?` for both rows). | functional | [#size-alignment-count](../../../docs/llm-generated/ir-reference/misc.md#size-alignment-count) | [`sizeof-alignof-data-layout-operand.slang`](sizeof-alignof-data-layout-operand.slang) |
| Passing an explicit data-layout type to `sizeof` selects the corresponding layout token (`Std140Layout` / `Std430Layout`) in the second operand of the `sizeOf` opcode. | functional | [#size-alignment-count](../../../docs/llm-generated/ir-reference/misc.md#size-alignment-count) | [`sizeof-explicit-data-layout.slang`](sizeof-explicit-data-layout.slang) |
| `alignof(T)` in a generic body lowers to the `alignOf` IR opcode with the type operand and an optional data-layout operand. | functional | [#size-alignment-count](../../../docs/llm-generated/ir-reference/misc.md#size-alignment-count) | [`alignof-generic.slang`](alignof-generic.slang) |
| `countof(D)` inside a `static const` field of a generic struct still surfaces `countOf` in the IR dump at LOWER-TO-IR. | functional | [#size-alignment-count](../../../docs/llm-generated/ir-reference/misc.md#size-alignment-count) | [`countof-struct-member.slang`](countof-struct-member.slang) |
| `countof(pack)` on a variadic int parameter pack lowers to the `countOf` IR opcode taking the pack as its operand. | functional | [#size-alignment-count](../../../docs/llm-generated/ir-reference/misc.md#size-alignment-count) | [`countof-pack.slang`](countof-pack.slang) |
| `sizeof(T)` in a generic body lowers to the `sizeOf` IR opcode carrying the type operand and a data-layout token. | functional | [#size-alignment-count](../../../docs/llm-generated/ir-reference/misc.md#size-alignment-count) | [`sizeof-generic.slang`](sizeof-generic.slang) |
| Two identical `getStringHash(...)` calls reference the same string-literal operand in the IR dump (stable hash, same operand). | functional | [#string-hashing](../../../docs/llm-generated/ir-reference/misc.md#string-hashing) | [`string-hash-deterministic.slang`](string-hash-deterministic.slang) |
| `getStringHash("...")` lowers to the `getStringHash` IR opcode carrying the literal string operand for a stable compile-time hash. | functional | [#string-hashing](../../../docs/llm-generated/ir-reference/misc.md#string-hashing) | [`string-hash.slang`](string-hash.slang) |
| The `is` operator on an existential parameter lowers to the `IsType` IR opcode with `(value, valueWitness, typeOperand, targetWitness)`. | functional | [#type-queries-and-predicates](../../../docs/llm-generated/ir-reference/misc.md#type-queries-and-predicates) | [`istype-existential.slang`](istype-existential.slang) |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| (unspecified) | undocumented-behavior | The `Pack and expansion` row for `Each` is listed with AST origin `(synthesized)`, but the natural surface for projecting one slot of a pack (`each D` inside an `expand` block) lowers to `getTupleElement(%pack, %index)` at LOWER-TO-IR rather than to the `Each` opcode the doc names. The doc could state that `Each` is introduced by a later variadic-specialization pass and is not produced by `slang-lower-to-ir.cpp` directly. |  |
| [#internal-core-module-only](../../../docs/llm-generated/ir-reference/misc.md#internal-core-module-only) | undocumented-behavior | The `Type queries and predicates` table claims `IsInt`, `IsBool`, `IsFloat`, `IsHalf`, `IsUnsignedInt`, `IsSignedInt`, `IsVector`, `IsCoopFloat`, and `TypeEquals` are observable, but the only source-level surfaces are the underscore-prefixed core-module intrinsics (`__isInt`, `__isBool`, ...). The doc could either name the user-visible surface (if any) or label the row "(internal / core-module only)". |  |
| [#runtime-array-surface-only](../../../docs/llm-generated/ir-reference/misc.md#runtime-array-surface-only) | undocumented-behavior | `GetArrayLength` is listed with `ArrayLengthExpr (runtime path)` as its AST origin. The natural surface (`arr.length()` on a fixed-size array) constant-folds at lowering and the opcode does not appear in the IR dump. The doc could state which surface prevents folding (a runtime-sized array binding) or label the row "(runtime-array surface only)". |  |
| (unspecified) | undocumented-behavior | The `Size, alignment, count` rows list `dataLayout?` as the second operand of `sizeOf` / `alignOf` but do not name the default token that fills the operand when the surface call passes only the type argument; from observation that token is `ScalarLayout`. | Naming the default would let readers predict the IR-dump shape without reading lowering code. |

## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| **`GetArrayLength`** — the natural `arr.length()` surface constant-folds at lowering on the fixed-size array path; the doc names a "runtime path" but does not name a portable surface that prevents folding. | (unclassified) | [#getarraylength](../../../docs/llm-generated/ir-reference/misc.md#getarraylength) | Not reachable via any allowed test directive. |
| **System opcodes** (`nop`, `Unrecognized`) — `nop` is not emitted by `slang-lower-to-ir.cpp`; `Unrecognized` only appears immediately after deserializing a module that uses an opcode the current build does not define. | (unclassified) | [#nop](../../../docs/llm-generated/ir-reference/misc.md#nop) | Not reachable via any allowed test directive. |
| **Type queries and predicates (remainder)** (`TypeEquals`, `IsInt`, `IsBool`, `IsFloat`, `IsCoopFloat`, `IsHalf`, `IsUnsignedInt`, `IsSignedInt`, `IsVector`) — only reachable through underscore-prefixed core-module intrinsics, not a user-portable surface. | (unclassified) | [#typeequals](../../../docs/llm-generated/ir-reference/misc.md#typeequals) | Not reachable via any allowed test directive. |
| **Kernel launch** (`DispatchKernel`, `CudaKernelLaunch`) — produced by host-side lowering for CUDA / host-shader targets; not reachable from a portable compute entry point on the `-target spirv-asm` path. | needs-unit-test | [#dispatchkernel](../../../docs/llm-generated/ir-reference/misc.md#dispatchkernel) | Not reachable via any allowed test directive. |
| **Tensor and runtime helpers** (`makeArrayList`, `makeTensorView`, `allocTorchTensor`, `TorchGetCudaStream`, `TorchTensorGetView`, `allocateOpaqueHandle`) — host-side runtime opcodes produced by non-shader lowering paths; not reachable from a portable compute entry point. | needs-unit-test | [#makearraylist](../../../docs/llm-generated/ir-reference/misc.md#makearraylist) | Not reachable via any allowed test directive. |
| **Annotations** (`Annotation`, `WitnessTableAnnotation`, `DifferentiableTypeAnnotation`, `DifferentiableTypeDictionaryItem`) — all `(synthesized)` by IR passes (notably the differentiation pipeline). | link-stage-only | [#annotation](../../../docs/llm-generated/ir-reference/misc.md#annotation) | Not reachable via any allowed test directive. |
| **Storage-type legalization casts** (`CastStorageToLogical`, `CastStorageToLogicalDeref`, `MakeStorageTypeLoweringConfig`, `CastUInt64ToDescriptorHandle`, `CastDescriptorHandleToUInt64`, `CastDescriptorHandleToResource`, `CastResourceToDescriptorHandle`, `TreatAsDynamicUniform`, `GetLegalizedSPIRVGlobalParamAddr`) — all `(synthesized)` by `slang-ir-lower-buffer-element-type.cpp` after LOWER-TO-IR. | link-stage-only | [#caststoragetological](../../../docs/llm-generated/ir-reference/misc.md#caststoragetological) | Not reachable via any allowed test directive. |
| **Pack and expansion (remainder)** (`Each`, `MakeWitnessPack`, `makeValuePack`, `PackBranch`, `ExtractFirstFromPack`, `ExtractLastFromPack`, `TrimFirstOfPack`, `TrimLastOfPack`, `ShapeConcat`, `ShapePermute`, `ShapeSwap`, `ShapeReduce`, `NonEmptyPackWitness`) — all `(synthesized)` per the doc; introduced by the variadic-generic specialization pass after LOWER-TO-IR. | link-stage-only | [#each](../../../docs/llm-generated/ir-reference/misc.md#each) | Not reachable via any allowed test directive. |
| **Liveness markers** (`liveRangeStart`, `liveRangeEnd`) — `(synthesized)` by `slang-ir-liveness.cpp`. | link-stage-only | [#liverangestart](../../../docs/llm-generated/ir-reference/misc.md#liverangestart) | Not reachable via any allowed test directive. |

## How to regenerate

```sh
python3 tests-agentic/_meta/regenerate.py regenerate ir-reference/misc
```
