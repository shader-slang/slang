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
`## Out of scope (no-GPU runner)` below.

## Claims enumerated

| Claim ID | Anchor                       | Claim (one line)                                                                                                                     | Tests                                                                                                                          |
| -------- | ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------ |
| C-01     | #size-alignment-count        | `sizeof(T)` on a generic type parameter lowers to `sizeOf(%T, ScalarLayout)` at LOWER-TO-IR.                                         | [`sizeof-generic.slang`](sizeof-generic.slang), [`sizeof-alignof-data-layout-operand.slang`](sizeof-alignof-data-layout-operand.slang)                                                             |
| C-02     | #size-alignment-count        | `alignof(T)` on a generic type parameter lowers to `alignOf(%T, ScalarLayout)` at LOWER-TO-IR.                                       | [`alignof-generic.slang`](alignof-generic.slang), [`sizeof-alignof-data-layout-operand.slang`](sizeof-alignof-data-layout-operand.slang)                                                            |
| C-03     | #size-alignment-count        | `countof(D)` on a variadic int-pack parameter lowers to the `countOf(%D)` opcode.                                                    | [`countof-pack.slang`](countof-pack.slang), [`countof-struct-member.slang`](countof-struct-member.slang)                                                                            |
| C-04     | #size-alignment-count        | `sizeOf` accepts an explicit data-layout type as its second operand (`Std140Layout` / `Std430Layout` token in the IR).               | [`sizeof-explicit-data-layout.slang`](sizeof-explicit-data-layout.slang)                                                                                            |
| C-05     | #type-queries-and-predicates | The `is` operator on an existential-typed parameter lowers to `IsType(value, valueWitness, typeOperand, targetWitness)`.             | [`istype-existential.slang`](istype-existential.slang)                                                                                                     |
| C-06     | #pack-and-expansion          | The `expand` keyword in a variadic-generic body lowers to the `Expand` IR opcode that introduces an iteration scope over a pack.    | [`expand-variadic-pack.slang`](expand-variadic-pack.slang)                                                                                                   |
| C-07     | #string-hashing              | `getStringHash("<literal>")` lowers to the `getStringHash` opcode carrying the string-literal operand verbatim.                      | [`string-hash.slang`](string-hash.slang), [`string-hash-deterministic.slang`](string-hash-deterministic.slang)                                                                         |

## Tests in this bundle

| File                                          | Intent     | Doc anchor                       |
| --------------------------------------------- | ---------- | -------------------------------- |
| [`sizeof-generic.slang`](sizeof-generic.slang)                        | functional | `#size-alignment-count`          |
| [`alignof-generic.slang`](alignof-generic.slang)                       | functional | `#size-alignment-count`          |
| [`countof-pack.slang`](countof-pack.slang)                          | functional | `#size-alignment-count`          |
| [`countof-struct-member.slang`](countof-struct-member.slang)                 | functional | `#size-alignment-count`          |
| [`sizeof-alignof-data-layout-operand.slang`](sizeof-alignof-data-layout-operand.slang)    | functional | `#size-alignment-count`          |
| [`sizeof-explicit-data-layout.slang`](sizeof-explicit-data-layout.slang)           | functional | `#size-alignment-count`          |
| [`istype-existential.slang`](istype-existential.slang)                    | functional | `#type-queries-and-predicates`   |
| [`expand-variadic-pack.slang`](expand-variadic-pack.slang)                  | functional | `#pack-and-expansion`            |
| [`string-hash.slang`](string-hash.slang)                           | functional | `#string-hashing`                |
| [`string-hash-deterministic.slang`](string-hash-deterministic.slang)             | functional | `#string-hashing`                |

## Doc gaps observed

- The `Pack and expansion` row for `Each` is listed with AST origin
  `(synthesized)`, but the natural surface for projecting one slot
  of a pack (`each D` inside an `expand` block) lowers to
  `getTupleElement(%pack, %index)` at LOWER-TO-IR rather than to
  the `Each` opcode the doc names. The doc could state that `Each`
  is introduced by a later variadic-specialization pass and is not
  produced by `slang-lower-to-ir.cpp` directly.
- The `Type queries and predicates` table claims `IsInt`, `IsBool`,
  `IsFloat`, `IsHalf`, `IsUnsignedInt`, `IsSignedInt`, `IsVector`,
  `IsCoopFloat`, and `TypeEquals` are observable, but the only
  source-level surfaces are the underscore-prefixed core-module
  intrinsics (`__isInt`, `__isBool`, ...). The doc could either
  name the user-visible surface (if any) or label the row
  "(internal / core-module only)".
- `GetArrayLength` is listed with `ArrayLengthExpr (runtime path)`
  as its AST origin. The natural surface (`arr.length()` on a
  fixed-size array) constant-folds at lowering and the opcode does
  not appear in the IR dump. The doc could state which surface
  prevents folding (a runtime-sized array binding) or label the
  row "(runtime-array surface only)".
- The `Size, alignment, count` rows list `dataLayout?` as the
  second operand of `sizeOf` / `alignOf` but do not name the
  default token that fills the operand when the surface call
  passes only the type argument; from observation that token is
  `ScalarLayout`. Naming the default would let readers predict the
  IR-dump shape without reading lowering code.

## Out of scope (no-GPU runner)

These opcodes are listed in `misc.md` but have no portable Slang
shader-language surface that reliably produces them at the LOWER-
TO-IR stage; they are introduced by later IR passes, host-side
lowering, or core-module reflection paths:

- **System opcodes** (`nop`, `Unrecognized`) — `nop` is not
  emitted by `slang-lower-to-ir.cpp`; `Unrecognized` only appears
  immediately after deserializing a module that uses an opcode the
  current build does not define.
- **Tensor and runtime helpers** (`makeArrayList`,
  `makeTensorView`, `allocTorchTensor`, `TorchGetCudaStream`,
  `TorchTensorGetView`, `allocateOpaqueHandle`) — host-side
  runtime opcodes produced by non-shader lowering paths; not
  reachable from a portable compute entry point.
- **Pack and expansion (remainder)** (`Each`, `MakeWitnessPack`,
  `makeValuePack`, `PackBranch`, `ExtractFirstFromPack`,
  `ExtractLastFromPack`, `TrimFirstOfPack`, `TrimLastOfPack`,
  `ShapeConcat`, `ShapePermute`, `ShapeSwap`, `ShapeReduce`,
  `NonEmptyPackWitness`) — all `(synthesized)` per the doc;
  introduced by the variadic-generic specialization pass after
  LOWER-TO-IR.
- **Type queries and predicates (remainder)** (`TypeEquals`,
  `IsInt`, `IsBool`, `IsFloat`, `IsCoopFloat`, `IsHalf`,
  `IsUnsignedInt`, `IsSignedInt`, `IsVector`) — only reachable
  through underscore-prefixed core-module intrinsics, not a
  user-portable surface.
- **`GetArrayLength`** — the natural `arr.length()` surface
  constant-folds at lowering on the fixed-size array path; the
  doc names a "runtime path" but does not name a portable surface
  that prevents folding.
- **Storage-type legalization casts** (`CastStorageToLogical`,
  `CastStorageToLogicalDeref`, `MakeStorageTypeLoweringConfig`,
  `CastUInt64ToDescriptorHandle`, `CastDescriptorHandleToUInt64`,
  `CastDescriptorHandleToResource`, `CastResourceToDescriptorHandle`,
  `TreatAsDynamicUniform`, `GetLegalizedSPIRVGlobalParamAddr`) —
  all `(synthesized)` by `slang-ir-lower-buffer-element-type.cpp`
  after LOWER-TO-IR.
- **Annotations** (`Annotation`, `WitnessTableAnnotation`,
  `DifferentiableTypeAnnotation`, `DifferentiableTypeDictionaryItem`)
  — all `(synthesized)` by IR passes (notably the differentiation
  pipeline).
- **Liveness markers** (`liveRangeStart`, `liveRangeEnd`) —
  `(synthesized)` by `slang-ir-liveness.cpp`.
- **Kernel launch** (`DispatchKernel`, `CudaKernelLaunch`) —
  produced by host-side lowering for CUDA / host-shader targets;
  not reachable from a portable compute entry point on the
  `-target spirv-asm` path.

## How to regenerate

```sh
python3 tests-agentic/_meta/regenerate.py regenerate ir-reference/misc
```
