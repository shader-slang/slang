# Slice 164: Fixed Float64 vector algebra

## Motivation

Two healthy frozen-v1 workloads stopped on the same selected-type boundary. Consider the reduced
Float64 vector workload:

```slang
typedef vector<double, 3> FloatVector;

FloatVector f = FloatVector(0.1f, vf, vf + 0.2f);
FloatVector ft = {};
FloatVector nv = normalize(f);
ft += nv;
ft += sign(f - 0.5l);
ft += saturate(f * 4 - 2.0l);
```

Numeric legalization preserves ordinary component-wise work as fixed `vector<double,3>` values.
The matrix workload follows the same representation after matrix legalization: a 2x2 matrix is an
`Array<Vec(Double, 2), 2>`, and its row algebra is fixed-vector work. Their exact first unsupported
shapes were respectively:

```text
add: vector<double,3> -> vector<double,3>
add: vector<double,2> -> vector<double,2>
```

`NVVMSemantics::isSelectedFloatValue` admitted one-to-four-lane Float16 and Float32 values but only
scalar Float64. Both workloads therefore failed deterministic Slang preflight before provider
creation even though the existing LLVM provider represents and emits fixed-vector floating
operations.

## Proposed solution

Make fixed Float64 vectors of two through four lanes ordinary selected floating values. Reuse the
existing generic typed operation catalog and provider callback: each operation still proves its
exact result and operand kinds, widths, lane/broadcast relationship, and family-specific contract.
Do not add a Float64-specific interface, provider callback, or ABI revision.

This is deliberately a representation widening rather than blanket vectorization. Operations with
scalar-only contracts, including the catalog's libdevice-backed minimum and maximum, remain
unsupported as vectors. Legalization already scalarizes those operations for the selected real
workloads. Mixed widths, unrelated aggregates, and vectors wider than four lanes also remain
deterministically rejected.

## Change summary

- The semantic catalog admits Float64 in its existing one-to-four-lane selected-float predicate.
- Real-provider numeric-family coverage serializes Float64x2 arithmetic, comparison, integer and
  floating conversion, and checks the five-lane and vector-minimum negative boundaries.
- A fake compiler fixture proves canonical Float64x2 algebra reaches exact typed descriptors; its
  validator now recognizes the existing 64-bit floating descriptor as Double.
- The two existing reduced Float64 workloads gain permanent direct-NVVM O0 and O3 differential
  lanes.
- Separate frozen and discovery census artifacts, 26-gate measurement input, design record,
  capability ledger, plan, and this report retain the validation evidence.

## Concepts and vocabulary

**Selected floating value** is the bounded scalar-or-fixed-vector representation that the shared
semantic catalog permits generic typed operations to consume. Selection alone does not authorize
an operation; the operation-family resolver must separately prove its full typed contract.

**Numeric legalization** rewrites source-level numeric forms into the canonical scalar and
fixed-vector operations seen by target emission. **Matrix legalization** represents a matrix as
fixed rows inside a fixed array, so its remaining arithmetic can reuse vector semantics.

**Scalar-only operation** means a semantic catalog row whose external or libdevice contract is
defined only for one lane. Supporting the element type does not implicitly create a vector ABI for
such an operation.

## Process report

The audit began at each exact first failure rather than at the fixture name. Final linked IR for
`vector-double-reduced-intrinsic.slang` contains ordinary Float64 vector add, subtract, multiply,
divide, comparisons, and integer-to-Float64 conversion. Final linked IR for
`matrix-double-reduced-intrinsic.slang` represents each row as `vector<double,2>` and retains row
add, subtract, multiply, and integer-vector conversion. Scalar-only source intrinsics such as
`abs`, `min`, `max`, `sign`, and reciprocal have already been decomposed into scalar helper loops.
The exact shape is therefore canonical and intentional, produced by numeric/matrix legalization;
it is not an alternative spelling that the direct emitter should repair.

`_getNVVMValueOperation` describes each final IR operation with `SlangNVVMValueTypeDesc` and calls
`NVVMSemantics::resolveValueOperationFamily`. That resolver first used
`isSelectedFloatValue`, whose Float64 scalar restriction was the only failed representation check.
After widening that predicate, the unchanged arithmetic and conversion family rules continue to
verify exact element widths, result kinds, operand counts, and vector/scalar broadcast rules. The
unchanged LLVM provider consumes the same descriptors in `_emitCatalogOperation` and emits native
fixed-vector LLVM instructions. This producer-to-consumer trace is why the semantic catalog is the
right ownership boundary and why no provider ABI change is justified.

The focused real-provider function exercises Float64x2 add, subtract, multiply, divide, remainder,
comparison, signed-integer conversion, and Float32/Float64 width conversion. Serialization checks
prove that LLVM receives `<2 x double>` operations. A five-lane add remains unsupported, and a
Float64x2 minimum remains unsupported because its catalog family is scalar-only. Removing the
single production widening restores the selected fixtures' original preflight failure, while these
negative checks show that it does not admit adjacent unproved shapes.

The first synthetic compiler fixture used ordinary scalar entry parameters. That reached the
separate unsupported launch-parameter ABI before it could test numeric emission, so it was rejected
as an invalid proof for this slice. The retained fixture uses the conventional dispatch-thread
entry ABI and an `RWStructuredBuffer<double>` output, isolating Float64x2 algebra and signed-i32x2
conversion. It exposed a test-only validator assumption that classified every non-half floating
vector as Float32. Mapping an existing 64-bit floating descriptor to the fake Double kind is not a
production fallback or a reconstructed type: it makes the fake validator honor the descriptor
that is already the semantic source of truth, and the focused fake test proves the path.

Both promoted workloads compare correctly against their stable CUDA/NVRTC references at O0 and O3.
Frozen corpus v1 retains exactly 452 workloads and 427 healthy references and advances from
394/398/394 to 396/400/396 O0/O3/both, with exactly these two gains and no old-correct loss.
Discovery retains exactly 82/72 and 64/64/64, also with no loss. The selected NVVM unit prefix
passes 433/433. The measurement manifest contains 26 gates and produced the expected 130 rows and
130 assembled cubins: direct O0 and NVRTC O3 at SM70 plus direct O3 at SM70, SM80, and SM90.

The new vector gate measured 278.0 ms and 3,538-byte PTX through direct O3 SM70 versus 429.6 ms and
11,410 bytes through NVRTC O3. The matrix-row gate measured 304.5 ms and 4,893-byte PTX versus
461.5 ms and 12,828 bytes. Direct O3 PTX contains the expected Float64 arithmetic and conversion
instructions and assembles with CUDA 12.9 across all three architectures. These one-repetition
measurements are exploratory, not controlled benchmark claims.

The final special-case inventory contains no new production helper, fallback, callback, or
shape-specific branch. The sole production change is the established selected-type predicate.
The fake Double classification survives because it corrects test infrastructure to match the
existing typed source of truth. No fixture-name check, syntax reconstruction, compatibility path,
downstream malformed-IR patch, or corpus reclassification was introduced.
