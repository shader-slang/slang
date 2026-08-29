# Add Float16 values and width-generic numeric conversions

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM accepts selected Float16 scalar and two- through four-lane vector
values, their ordinary arithmetic and comparisons, and lane-preserving numeric conversions among
selected integers, Float16, and Float32. The complete existing Float builtin-operator fixture
should pass its direct runtime/PTX lanes instead of stopping at `floatCast`, and representative
existing half arithmetic should run through the same generic typed-operation interface.

## Progress

- [x] (2026-08-29) Reproduced the post-Slice-93 boundary: the complete Float fixture stops at
  Float32-to-Float16 `floatCast`; scalar half arithmetic first stops at integer-to-Float16
  `castIntToFloat`; vector half arithmetic stops at vector `floatCast`.
- [x] (2026-08-29) Confirmed final IR uses canonical `Half`, `Vec(Half,N)`, `floatCast`, ordinary
  arithmetic/comparison ops, helpers, phis, and the existing vector construction/extraction path.
- [x] (2026-08-29) Confirmed the CUDA 12.9 NVVM IR 2.0 type system admits `half`; CUDA 12.2 and
  earlier specifications did not, so this slice must record the resulting toolkit floor rather
  than pretending older libNVVM releases have the same contract.
- [x] (2026-08-29) Generalized the semantic type and operation catalog to selected Float16/Float32
  widths and lane-preserving integer/floating conversions under exact forward-only builder ABI
  revision 10.
- [x] (2026-08-29) Lowered and validated Float16 scalar/vector types, constants, operations,
  helper/control-flow values, and conversions through the real and fake providers.
- [x] (2026-08-29) Added focused positive/negative builder and emitter coverage plus file-backed
  runtime/PTX evidence for scalar/vector arithmetic, comparison, conversion, and helper/phi
  transport.
- [x] (2026-08-29) Formatted, built, ran focused/full/CUDA validation, assembled PTX,
  self-reviewed, updated durable docs and this plan, and prepared the complete slice for commit.

## Surprises and Discoveries

- CUDA 12.9 changed the NVVM IR type-system contract: `half` is no longer in the unsupported
  floating-point list. Earlier specifications, including CUDA 12.2, explicitly reject it. Native
  LLVM `half`, `fptrunc`, and `fpext` are therefore a current-toolkit capability with a real
  compatibility floor, not a timeless LLVM-7 property.
- The current provider descriptor already carries floating-point bit width and lane count, and the
  generic operation callback already owns arithmetic/comparison/conversion dispatch. No Float16
  callback family is needed; the bring-up restrictions are selected-width predicates and the
  missing floating-to-floating operation ID.
- LLVM 14 prints its native unary floating negation as `fneg`, which the libNVVM LLVM 7 reader
  rejects. Building typed `fsub -0.0, value` in the LLVM graph preserves both Half and Float
  vector types and avoids reintroducing the text-level type reconstruction removed in Slice 68.
- The fake provider's vector extractor originally enumerated Integer, Boolean, and Float vectors
  even after generic construction admitted Half. The focused emitter test exposed that exact
  test-double omission; adding Half to the same type-driven enumeration fixed it without a
  production special case.
- CUDA 12.9 libNVVM accepts this slice's scalar Half module at its default optimization level, but
  rejects the representative Half2-heavy module with the generic `unsupported operation` result
  at O0. The same verified module compiles and runs at O3. The file-backed Half2 lane therefore
  records an explicit optimization requirement rather than changing the semantic representation.
- Re-probing adjacent fixtures after implementation found two independent next boundaries at both
  O0 and O3: `half-vector-calc.slang` reaches mutable local `var`, while
  `half-vector-compare.slang` stops at the `Values` helper result type. Numeric Half operations no
  longer mask either boundary.

## Decision Log

- Decision: represent selected Half values as native LLVM `half` under the CUDA 12.9 NVVM IR 2.0
  contract.
  Rationale: this preserves Slang's canonical type and rounding points directly. Promoting every
  operation to Float32 or representing Half as i16 would change semantics or require older
  conversion intrinsics and would conceal the current provider capability.
  Date/author: 2026-08-29, Codex.
- Decision: add one `FLOAT_CONVERT` typed operation and generalize existing numeric conversion
  families across matching lane counts.
  Rationale: result and operand descriptors already encode kind, width, signedness, and lanes.
  Per-width or per-direction callbacks would repeat the scaling problem removed in earlier slices.
  Date/author: 2026-08-29, Codex.
- Decision: include Float16 arithmetic, comparison, vector construction/extraction, helpers, and
  phis in the same slice.
  Rationale: these are all consequences of admitting Float16 as one selected first-class semantic
  type. Splitting each operation would create artificial micro-slices without an architectural
  decision boundary.
  Date/author: 2026-08-29, Codex.
- Decision: lower generic floating negation to typed LLVM `fsub` from negative zero.
  Rationale: `fneg` is not in the LLVM 7 dialect consumed by libNVVM, while typed subtraction is
  accepted, preserves scalar/vector element types, and stays inside the provider's LLVM graph.
  Date/author: 2026-08-29, Codex.
- Decision: require O3 only on the new Half2-heavy file-backed lane and retain the default-level
  scalar Half lane.
  Rationale: this makes the measured CUDA 12.9 libNVVM limitation visible without globally
  requiring optimization or weakening the native Half value contract.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Builder ABI revision 10 now admits native LLVM Half scalar/vector values through the existing
generic type, function, control-flow, aggregate-element, and operation interfaces. One
`FLOAT_CONVERT` semantic operation covers Float16/Float32 width changes, while the existing
integer/floating conversion families now preserve matching vector lanes. Half constants,
arithmetic, comparisons, helper calls/results, phis, construction, dynamic extraction, and every
conversion direction are validated by both real and fake providers.

The complete Float builtin-operator fixture now passes direct CUDA runtime and PTX lanes. The new
Half fixture returns `-8, -5, 1, -5`, emits 1,608 bytes of PTX containing native f16/f16x2 forms,
and assembles with `ptxas -arch=sm_70` to a 3,176-byte cubin. The scalar `half-calc.slang` fixture
continues to prove default-optimization execution; the Half2-heavy lane explicitly uses O3 because
CUDA 12.9 libNVVM rejects that module at O0.

The next implementation boundary is no longer numeric typing. Mutable local SSA/storage lowering
blocks `half-vector-calc.slang`, while the stateful `Values` helper ABI blocks
`half-vector-compare.slang`; those should be planned as storage/control-flow work rather than
special-cased as more Half operations.

## Context and Current Pipeline

Consider the retained section of the complete Float fixture:

    half h = half(outputBuffer[13]) + half(2.0);
    outputBuffer[13] = h + a;
    int3 i3 = int3(1, 2, 3);
    float3 mv = i3 + float3(a, b, a);

Before this slice, final IR expressed this as Float32-to-Half `floatCast`, Half `add`,
Half-to-Float32 `floatCast`, and an Int3-to-Float3 `castIntToFloat`. Direct preflight failed on the
first `floatCast`. The provider type descriptor could already spell
`{ FLOATING_POINT, 16, lanes }`, but the shared semantic catalog accepted only width 32, the
provider type map created only LLVM `float`, and the emitter recognized only Float32 constants and
scalar validation.

Exact ABI revision 10 keeps the same single generic `emitOperation` callback. It adds the
semantic `FLOAT_CONVERT` operation ID and admits Float16 wherever the selected first-class
scalar/vector value role is intended. Memory/resource policies remain Float32-only unless a test
and layout contract explicitly require otherwise.

## Scope and Non-Goals

In scope:

- native LLVM/NVVM `half` scalar and fixed-vector value types;
- Float16 literals, ordinary add/subtract/multiply/divide/remainder/negate, and all six comparisons;
- explicit Float16-to-Float32 and Float32-to-Float16 conversion, scalar or same-lane vector;
- selected integer-to-Float16/Float32 and Float16/Float32-to-selected-integer conversions with
  matching scalar/vector lanes;
- Float16 helper parameter/result, phi/branch transport, vector construction/extraction, and
  exact fake/real-provider validation;
- direct runtime/PTX evidence on CUDA 12.9 plus the complete existing Float fixture.

Out of scope:

- Float64, BFloat16, FP8, mixed-lane conversions, saturation/rounding-mode flags, or implicit
  bitcasts;
- Half pointers, structured/byte-address storage, conventional parameters, global fields, matrix
  values, wave intrinsics, atomics, or libdevice math;
- mutable local aggregate storage and pointer/reference helper ABI, including the independent
  `Values` boundary in `half-vector-compare.slang`;
- compatibility with libNVVM versions whose NVVM IR specification rejects native `half`.

## Architecture and Invariants

- Slang's final canonical scalar/vector type is the source of truth for semantic kind, width, and
  lane count. The emitter does not infer Half from operation names or literal values.
- One descriptor operation maps each accepted semantic to LLVM. Float conversion is lane
  preserving and changes width; integer/floating conversions preserve lanes and use signedness
  only where the source or result integer requires it.
- Every provider operation validates descriptor support, exact original operand types, insertion
  availability, and result type before mutation. Unsupported widths, lanes, or mixed shapes fail
  without an output handle.
- Float16 values are first-class SSA values. They are not silently promoted between operations,
  spilled, or reconstructed from integer syntax.
- The compiler's memory and entry ABI role classifiers stay narrow. Admitting Half as a value does
  not implicitly admit Half storage or raw parameters.

## Interfaces and Dependencies

Advance `SLANG_NVVM_BUILDER_ABI_REVISION` to 10 because the operation enum/count and negotiated
semantic contract change. Add `SLANG_NVVM_VALUE_OP_FLOAT_CONVERT`; add selected Float16 predicates
and constants to `slang-nvvm-semantic-catalog.h`; update the facade, real LLVM provider, fake
provider, direct type lowering, preflight, validation, and emission together. No public Slang API
or libNVVM API changes.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. Builds and tests run outside the sandbox per repository instructions.

## Milestones

1. Generalize selected floating semantic descriptors and conversion families; cover descriptor
   acceptance/rejection without changing the callback topology.
2. Materialize LLVM Half types/constants and emit native half arithmetic, comparison, `fptrunc`,
   `fpext`, and integer/floating conversions in normal and NVVM-2.0-compatible text.
3. Admit canonical Half scalar/vector values in direct type lowering, literals, helpers, phis,
   vector construction/extraction, and ordinary typed-operation emission while retaining narrow
   storage/entry roles.
4. Add focused fake/real-provider tests and file-backed runtime/PTX lanes, then measure the next
   independent boundary in representative existing Half suites.
5. Format, build, run the complete NVVM prefix and changed shader prefixes, assemble PTX, perform
   the input-shape/special-case audit, update durable documents and this plan, and commit.

## Validation and Acceptance

Acceptance requires normal and compatible provider assembly with native `half`, Half arithmetic
and comparison, `fptrunc`/`fpext`, vector numeric conversions, negative no-mutation coverage, fake
descriptor/type/value traces, direct runtime and PTX lanes, CUDA 12.9 `ptxas`, the complete
`slang-unit-test-tool/nvvm` prefix, pinned clang-format 17, and `git diff --check`.

Completed evidence on 2026-08-29:

- Release `slangc`, `slang-unit-test`, and `slang-test` host targets built successfully; the
  standalone `slang-llvm-nvvm` Release provider also built successfully.
- `slang-unit-test-tool/nvvm`: 366/366 passed with the standalone real provider configured.
- `tests/cuda/nvvm-half-values`: 2/2 passed; runtime output was `-8, -5, 1, -5`.
- `tests/compute/half-calc`: 3/3 available lanes passed and one unavailable D3D12 lane was ignored.
- `tests/language-feature/operator-overload/builtin-operator-fastpath-float`: 4/4 available lanes
  passed and one unavailable D3D12 lane was ignored.
- The final optimized Half PTX was 1,608 bytes and `ptxas -arch=sm_70` produced a 3,176-byte cubin.
- The pinned clang-format 17 completed on every changed C++/header file, and `git diff --check`
  reported no errors.

## Self-Review and Input-Shape Audit

The new production helpers are `isNVVMFloat16Type` and
`isNVVMSupportedFloatingPointScalarType`. They survive because they classify canonical linked-IR
types once at the type-lowering boundary; they do not reconstruct types, walk operand graphs, or
duplicate a semantic value. The expanded fake-provider value checker also survives because it
asserts exact provider contracts in tests and has no production behavior.

`kIROp_FloatCast` is handled in preflight, SSA availability, and emission through the same
descriptor resolver as every other numeric conversion. Its producer supplies canonical result and
operand types, so no operation-name, literal-value, or alternate-shape matching was added. The
provider's typed `fsub` choice is the LLVM-to-libNVVM dialect boundary and is covered by normal and
compatible assembly tests; it does not rewrite serialized text. No new fallback, silent default,
syntax reconstruction, arbitrary graph walk, or downstream repair remains in the diff.

## Failure and Recovery

If libNVVM rejects verified compatible text containing native Half, record the exact verifier or
compile diagnostic and reconsider an i16 plus conversion-intrinsic physical representation as a
new architecture decision; do not paper over it with textual substitution. Generated probes stay
under ignored `build/`. All exact-ABI host/provider changes land atomically in this slice. Never
reset unrelated work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Record exact descriptor rows, normal/compatible LLVM forms, fake traces, runtime output,
PTX/cubin sizes, full/focused test counts, the next suite stop, toolkit-floor evidence, and the
self-review inventory here. Distill the settled Float16 policy into `docs/design/nvvm-backend.md`
and durable evidence into the capability ledger.
