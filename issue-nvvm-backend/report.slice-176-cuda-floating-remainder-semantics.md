# Slice 176: CUDA floating-point remainder semantics

## Motivation

The frozen `hlsl-intrinsic/matrix-float.slang` workload compiled through direct NVVM but disagreed
with native CUDA in both O0 and O3. A reduced checkpoint around the first matrix expression showed
the exact divergence:

```slang
value += FloatMatrix(IntMatrix(((f % splat(0.11f)) * splat(100)) + splat(0.5f)));
```

Matrix construction, lane transport, and the preceding arithmetic all agreed. The fourth lane is
near a multiple of `0.11f`; direct emission used LLVM `frem`, while the CUDA route uses device
`fmodf`, and the different remainder changed the subsequent rounded integer by 22 before later
matrix arithmetic amplified it.

## Proposed solution

Classify canonical Float32/64 `kIROp_FRem` as a compiler-owned CUDA remainder recipe. Prove the
existing component-wise scalar/vector type relation, including scalar broadcast, apply the current
scalar `SLANG_NVVM_VALUE_OP_FMOD` operation to each result lane, and reconstruct the exact selected
vector. Record the scalar operation's libdevice requirement during the same preflight pass.

Keep integer remainder and the provider's generic LLVM floating remainder operation unchanged.
The revision-32 provider already supports scalar FMOD, sequential extraction, and vector
construction, so no ABI revision or new callback is necessary.

## Change summary

- Direct NVVM preflight and emission share one exact floating-remainder resolver.
- Vector emission extracts only vector operands and reuses scalar broadcast operands per lane.
- Requirement collection records both scalar FMOD and the module-level libdevice dependency.
- The vector-operation fake test now proves one Float3 `%` emits three scalar FMOD operations.
- `matrix-float.slang` gains permanent direct O0/O3 differential lanes.
- Frozen/discovery TSV and Pareto JSON, the measurement manifest, design notes, capability ledger,
  plan, and this report retain the validation evidence.

## Concepts and vocabulary

**Component-wise relation** is the established semantic family in which a selected vector result
accepts vector operands with the same lanes and scalar operands of the same element type as
broadcast values.

**Module-level libdevice requirement** tells the NVVM compiler to add the selected toolkit's
libdevice bitcode before compilation. A typed FMOD descriptor alone describes emission but does not
load its implementation.

**Selected vector** is the bounded scalar-vector representation accepted by the direct provider
ABI after Slang matrix legalization has flattened component-wise matrix arithmetic.

## Process report

Ordinary arithmetic lowering produces binary `kIROp_FRem`. CUDA matrix legalization flattens the
2x2 matrix operation to a selected Float4 value before direct emission. `_getNVVMValueOperation`
previously mapped both `kIROp_IRem` and `kIROp_FRem` to
`SLANG_NVVM_VALUE_OP_REMAINDER`; the provider's floating family implements that descriptor with
LLVM `IRBuilder::CreateFRem`. The exact input shape is canonical and intentionally valid. Its
producer accurately represents source `%`; the target-specific emitter owns the choice of CUDA
operation, so changing the producer or reconstructing source syntax would be the wrong layer.

`_resolveNVVMFloatingRemainderOperation` now first asks the shared semantic catalog to prove the
existing component-wise floating remainder relation. This reuses the established source of truth
for scalar/scalar, vector/vector, and vector/scalar broadcast rather than inventing another type
matrix. It then narrows the leaf to Float32 or Float64 and creates the existing scalar FMOD recipe
step. Capability collection and emission call this same resolver, preventing admitted and emitted
shapes from drifting.

`_emitNVVMFloatingRemainderOperation` directly emits a scalar result. For a selected vector, it
extracts one lane from every vector operand, reuses any scalar operand as the canonical broadcast,
emits scalar FMOD, and reconstructs the exact lowered vector. Matrix knowledge is absent from the
provider boundary: matrix legalization has already selected the ordinary vector representation,
and existing generic builder operations express the entire recipe.

The first complete frozen replay exposed a representation cascade. `matrix-float` still passed
because its many later math intrinsics independently requested libdevice, but `%`-only workloads
produced PTX with an unresolved `__nv_fmodf`. Requirement collection had added the FMOD descriptor
without setting `requirements.requiresCUDADeviceLibrary`. The same replay also exposed
`vector-float`'s canonical vector/scalar shape because the first resolver draft required identical
operand types. Fixing the common resolver to use the catalog's component-wise relation and setting
the module dependency at requirement collection restored all three prior successes. A bounded
four-workload native/O0/O3 probe passed 4/4 before the exact direct lanes were replayed.

The self-review inventory contains two production helpers, one test-fixture helper, and one
bounded switch special case. The resolver survives because removing it restores the measured
LLVM-`frem` runtime mismatch, and it delegates type ownership to the shared semantic catalog. The
emitter survives because the existing provider has only scalar libdevice FMOD and generic lane
operations; removing scalarization cannot express the selected vector semantics. The
`kIROp_FRem` switch cases survive because this canonical Slang operation now has a multi-operation
recipe rather than a one-descriptor mapping. The test-only helper constructs the same coherent
toolkit/libdevice filesystem shape already used by libdevice emitter tests; without it, the two
pre-existing fake sources containing `%` correctly fail production preflight before fake program
creation. None walks an arbitrary operand graph, rebuilds syntax, checks a fixture name, weakens a
diagnostic, patches malformed upstream IR, or introduces a compatibility fallback.

Frozen corpus v1 remains exactly 452 workloads and 427 healthy MVP references. Healthy correctness
advances from 413/413/413 to 414/414/414 O0/O3/both. The only classification changes are
`hlsl-intrinsic/matrix-float.slang#cuda-1` from runtime mismatch to correct in each direct mode;
there are zero old-correct regressions. All-row direct totals are 428 correct, three runtime
mismatches, and 21 preflight failures per mode.

Discovery remains exactly 82 workloads and 72 healthy references at 72/72/72, with identical IDs
and zero classification changes. The selected regression prefix passes 433/433 and the permanent
`nvvm` category passes 82/82. The representative matrix gate compiles and assembles through CUDA
12.9 for native NVRTC, direct O0 SM70, and direct O3 SM70/SM80/SM90. At SM70, standalone median
compile time is 917.6 ms native, 547.1 ms direct O0, and 609.6 ms direct O3; PTX sizes are 109,591,
321,704, and 104,640 bytes respectively. These single-repetition measurements remain exploratory.
