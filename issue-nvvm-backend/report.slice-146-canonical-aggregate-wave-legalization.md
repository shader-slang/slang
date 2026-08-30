# Slice 146 report: canonical aggregate wave legalization

## 1. Motivation

The Slice 145 healthy-MVP Pareto had four tied eight-workload clusters. Exact decomposition of the
wave/reconvergence cluster showed a larger shared producer family across the complete corpus: 20
aggregate shuffle/reduction/prefix workloads and two active-mask workloads. Prefix count and two
rotate workloads had different scalar semantics and were deliberately excluded.

Consider this source example:

```slang
WaveMask mask = 0xff;
matrix<int, 2, 2> value = matrix<int, 2, 2>(1, 2, 3, 4);
matrix<int, 2, 2> shuffled = WaveMaskReadLaneAt(mask, value, lane);
matrix<int, 2, 2> reduced = WaveMaskSum(mask, value);
```

The matrix overloads in `source/slang/hlsl.meta.slang` select CUDA-prelude assembly
`_waveShuffleMultiple($0, $1, $2)` and `_waveSumMultiple($1.x, $0)`. The direct-NVVM pipeline asks
`legalizeMatrixTypes` to lower every matrix to fixed arrays of row vectors. The final helper ABI is
therefore intentionally, for example,
`Void(uint, array<vector<int,2>,2>, int, OutParam<array<vector<int,2>,2>>)`. That is canonical IR,
not an emitter accident to patch or source syntax to reconstruct.

The existing direct backend already knew each required 32-bit scalar wave operation. It rejected
the aggregate helper because it had no representation for applying one scalar recipe recursively
and transporting the complete result through either a value result or the lowered matrix out
parameter.

## 2. Proposed solution

Represent the bounded CUDA-prelude family with one compiler-owned aggregate-wave descriptor:

- exact final assembly and complete specialized signature select shuffle, masked scan, or active
  mask semantics;
- one recursive type proof accepts only homogeneous vectors and fixed arrays with selected 32-bit
  signed, unsigned, or floating leaves;
- shuffle applies the established scalar read-lane-at operation to each leaf;
- reductions and prefixes reuse the established scalar masked-wave descriptor and compact loop for
  each leaf, then reconstruct the exact aggregate;
- value-return helpers return the aggregate, while exact `OutParam<T>` helpers store `T` and return
  void;
- `__activemask()` and `make_uint4(__activemask(), 0, 0, 0)` use full-mask ballot of `true` and
  ordinary vector construction;
- every typed operation is collected before provider discovery, including libdevice demand for
  floating min/max.

The provider ABI remains revision 30. Existing generic value, aggregate, control-flow, phi,
pointer, store, and return operations express the complete graph.

## 3. Change summary

- `source/slang/slang-emit-nvvm.cpp`
  - unifies scalar and aggregate masked-wave spellings in one exact table;
  - factors scalar masked-wave value emission from function-return transport;
  - validates and emits homogeneous vector/fixed-array wave operations recursively;
  - supports exact scalar and `uint4` converged-mask helpers;
  - defers aggregate-loop phi edges until the complete helper CFG is terminated;
  - propagates aggregate floating min/max libdevice requirements.
- `tools/slang-unit-test/unit-test-nvvm-support.h` and
  `tools/slang-unit-test/unit-test-nvvm-emitter.cpp`
  - add a focused source covering value-return vectors, lowered-matrix out transport, floating
    prefix min, and both active-mask results;
  - prove the emitted scalar operation counts, recursive construction, out stores, phi loops, and
    one lazy libdevice module;
  - add a malformed aggregate mask/signature neighbor that fails before provider discovery;
  - widen only the fake provider's bounded phi and intrinsic recording arrays needed by the larger
    focused graph.
- Nineteen existing workload files
  - add explicit direct-NVVM O0 and O3 comparison lanes.
- `issue-nvvm-backend/census.slice-146.tsv` and
  `issue-nvvm-backend/census.slice-146-clusters.json`
  - retain all 452 workload results and the post-slice Pareto distribution.
- `docs/design/nvvm-backend.md` and
  `docs/design/nvvm-backend-capability-ledger.md`
  - record the durable aggregate-wave contract, denominators, metrics, and remaining boundary.

The promoted workloads are the ten `wave-mask` files `wave-broadcast-lane-at`, `wave-get-active`,
`wave-get-converged`, `wave-mask-prefix-min-max`, `wave-matrix`, `wave-prefix-product`,
`wave-prefix-sum`, `wave-read-lane-at`, `wave-shuffle`, and `wave-vector`; plus `wave-matrix`,
`wave-prefix-bitwise`, `wave-prefix-min-max`, `wave-prefix-product`, `wave-prefix-sum`,
`wave-read-lane-at`, and `wave-vector`; plus `wave-multi-bitwise` and
`wave-multi-prefix-bitwise`.

## 4. Concepts and vocabulary

- **Aggregate wave algebra:** The finite recursive set of homogeneous selected-32-bit vectors and
  fixed arrays to which one already-validated scalar wave operation is applied leaf by leaf.
- **Masked scan:** The shared reduction/exclusive-prefix/inclusive-prefix loop over the bits of one
  `uint4` partition mask's first word.
- **Result transport:** Whether the canonical helper returns `T` directly or writes the same `T`
  through an exact `OutParam<T>` after matrix legalization.
- **Healthy MVP denominator:** The 427 MVP workloads whose NVRTC O3 reference succeeds on this
  machine. It excludes three native-reference infrastructure failures and 22 extension rows.
- **Selected regression prefix:** The focused direct-NVVM unit/integration suite. Its 424/424 score
  protects established behavior but is not the backend coverage denominator.

## 5. Process report

### The bounded population was audited by producer and final signature

The Slice 145 TSV first grouped 25 wave/reconvergence failures. Final assembly and signature split
them into 20 aggregate operations, two converged masks, one prefix-count operation, and two rotate
operations. The 22 selected rows came from overloads in `hlsl.meta.slang`, CUDA target
specialization, and the normal linked-IR legalization pipeline. No fixture path or source function
name participates in resolution.

The exact admitted transports were observed rather than guessed. Vector reductions and prefixes
normally produce `T(T, uint4)`. `legalizeMatrixTypes`, enabled for direct NVVM in
`source/slang/slang-emit.cpp`, lowers matrices to arrays of row vectors and uses
`Void(T, ..., OutParam<T>)`. `_resolveNVVMAggregateWaveOperation` validates the entire relation:
assembly, arity, mask/lane semantics, recursively homogeneous value type, exact result type, and
exact out pointee. `_isExactNVVMAggregateWaveOutParameter` does not accept an adjacent pointer role
or merely layout-compatible type.

The malformed focused neighbor changes the explicit mask from UInt to Int while retaining matrix
out transport. Removing the signature checks makes that test reach provider discovery; with the
checks it retains E52017 and zero provider mutation. This proves the direct emitter owns validation
of the exact target helper contract, not arbitrary generic assembly.

### One scalar recipe remains the semantic source of truth

`_initializeNVVMMaskedWaveScalarOperation` constructs identities, bit iteration, source-lane read,
typed combine, prefix predicate, and selection once. Scalar helpers and aggregate leaves share it.
`_getNVVMHomogeneousWaveAggregateLeafType` only classifies recursive vectors/fixed arrays; it does
not assign wave semantics itself.

`_emitNVVMAggregateMaskedWaveValue` extracts each canonical element, invokes
`_emitNVVMMaskedWaveScalarValue`, and rebuilds the original type. Each scalar loop starts in the
previous leaf's exit block. The provider requires the CFG to be complete before incoming phi edges
are attached, so the emitter records `NVVMMaskedWavePendingPhi` entries, performs the final
return/store, and then attaches all edges. This preserves the provider invariant without unrolling
the wave or changing upstream IR.

The first matrix probe exposed a Release-only implementation error: a semantic-type query had been
placed inside `SLANG_ASSERT`, so Release never executed it. The final code uses ordinary checked
control flow, and no temporary assertion remains.

### Active masks reuse ballot rather than extending the ABI

`WaveGetConvergedMask` and `WaveGetConvergedMulti` in `hlsl.meta.slang` produce exact
`__activemask()` and `make_uint4(__activemask(), 0, 0, 0)` helpers. `_emitNVVMActiveMaskValue`
constructs the already-established semantic equivalent `ballot_sync(0xffffffff, true)`. Scalar
transport returns the ballot word; vector transport constructs `uint4(mask, 0, 0, 0)`. A provider
callback for CUDA spelling would add no expressive power and was rejected.

### Floating aggregate min/max must retain libdevice ownership

The first successful compilation of the two prefix-min/max workloads produced invalid PTX at
module load. Their recursive Float32 leaves use the generic typed min/max family, which resolves to
libdevice. Scalar intrinsic recipes already propagate this fact, but masked-wave requirements did
not. `_initializeNVVMMaskedWaveScalarOperation` now records the exact combine operation's catalog
or family requirement, and both scalar and aggregate preflight propagate it before provider
discovery. The focused test observes one lazy module, and both real workloads then compare correctly
at O0 and O3. Adding an unconditional libdevice dependency was rejected because integer and
ordinary floating add/product waves do not require it.

### The three incomplete rows expose the next independent invariant

Nineteen of the 22 bounded workloads are correct in both modes. The remaining three are
multi-operation fixtures whose aggregate operations now compile and execute far enough to expose
their next exact unsupported shape:

- `_waveMin($1.x, $0)` with `double(double, vector<uint,4>)`;
- `_wavePrefixSum($1.x, $0) + $0` with `double(double, vector<uint,4>)`;
- `_waveSum($1.x, $0)` with `double(double, vector<uint,4>)`.

Float64 scalar wave transport is outside the selected homogeneous-32-bit aggregate invariant. The
emitter retains the exact diagnostic instead of broadening this slice or reporting those fixtures
as aggregate failures.

### Validation evidence and measured breadth

The fixed census remains 448 eligible sources and 452 eligible workloads:

| Mode | Correct | Runtime mismatch | Slang preflight | Provider | Infrastructure |
|---|---:|---:|---:|---:|---:|
| NVRTC O3 | 449 | 0 | 0 | 0 | 3 |
| direct NVVM O0 | 384 | 8 | 53 | 7 | 0 |
| direct NVVM O3 | 389 | 8 | 53 | 2 | 0 |

Compared with Slice 145, direct O0 and O3 each gain exactly 19 workload identities and lose zero
previously correct identities. On the 427 healthy-MVP denominator:

| Measure | Slice 145 | Slice 146 |
|---|---:|---:|
| O0 correct | 363/427 (85.0%) | 371/427 (86.9%) |
| O3 correct | 367/427 (85.9%) | 375/427 (87.8%) |
| correct in both modes | 363/427 (85.0%) | 371/427 (86.9%) |

All eight healthy-MVP wave/reconvergence failures are removed. Its six remaining corpus rows are
extension-only. The leading healthy-MVP clusters are now aggregate/pointer/layout, helper ABI/type,
and other exact preflight shapes at eight each; function identity at six; and raw-buffer access,
GenericAsm atomics, descriptor runtime layout, and atomic/wave operations at three each. O0 also
has four unoptimized-Half provider failures. Every smaller cluster remains in the checked-in TSV
and JSON.

The selected prefix passes 424/424. The 19 promoted files pass 57/57 CUDA lanes: one native NVRTC
lane and two direct O0/O3 lanes per file. Representative standalone measurements are:

| Workload gate | NVRTC O3 median / PTX | direct O3 SM70 median / PTX |
|---|---:|---:|
| resource + aggregate + helper | 408.1 ms / 8889 B | 269.1 ms / 919 B |
| parameter-block layout | 375.5 ms / 8839 B | 250.0 ms / 793 B |
| shared control + barriers | 381.9 ms / 9190 B | 259.0 ms / 1404 B |

Their census end-to-end NVRTC/direct-O3 compile-load-run-compare times are 5263/5145 ms,
4896/5139 ms, and 5200/5322 ms; these are not kernel-only timings. Direct O3 PTX assembles with
CUDA 12.9 for SM70, SM80, and SM90. Runtime comparison remains on the local SM120 GPU. CUDA 13 and
physical SM70/SM80/SM90 runtime workers remain explicit productionization gaps.

### Self-review inventory

- The shared spelling table survives. Every row comes from one CUDA-prelude producer family, is
  coupled to a complete signature, and replaces the previous duplicated scalar mapping.
- `_getNVVMHomogeneousWaveAggregateLeafType` survives. It follows canonical post-matrix-lowering
  structure and rejects heterogeneous or unsupported leaves; matrix and vector tests fail without
  it.
- `_isExactNVVMAggregateWaveOutParameter` survives. Matrix legalization intentionally produces
  this transport, and the malformed-signature test proves the emitter owns its validation.
- `_emitNVVMMaskedWaveScalarValue` and deferred phi records survive. They factor the established
  scalar loop from result transport and satisfy the provider's completed-CFG requirement.
- Active-mask ballot construction survives. It is the established target-independent semantic
  operation for the exact CUDA helper and needs no provider callback.
- Libdevice propagation survives. Float32 min/max PTX fails to load without it, while the focused
  lazy-module assertion proves the dependency remains demand-driven.
- The fake provider's recording arrays grow from 8 to 32 only to observe the focused recursive
  graph; no production limit or behavior changes.
- No compatibility fallback, fixture-name condition, arbitrary operand walk, syntax
  reconstruction, malformed-IR repair, or provider ABI revision was added.
