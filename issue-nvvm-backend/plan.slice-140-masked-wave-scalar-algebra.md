# Lower masked scalar wave algebra through generic builder operations

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires this experimental branch's slice plan to be committed with its implementation, which is
an exception to the repository's default active-plan lifetime policy.

## Purpose and Observable Result

After this slice, the direct NVVM path accepts the canonical CUDA-specialized scalar masked-wave
reduction and prefix helpers as one finite algebra instead of adding one fixture or intrinsic at a
time. Exact `IRGenericAsm` spellings and complete specialized signatures select a reduction or
prefix recipe. The compiler lowers each recipe through existing scalar value operations, wave
read-lane-at, constants, vector extraction, and select; no source assembly reaches the provider and
the revision-29 ABI remains unchanged.

The shortest observable result is that the healthy MVP workloads currently blocked first by
`_waveSum`, `_waveProduct`, `_waveMin`, or `_wavePrefixSum` become differentially correct at O0 and
O3. The fixed 452-workload census must show the exact gain and any later blockers. Every newly
correct fixture receives explicit direct O0 and O3 lanes.

## Progress

- [x] (2026-08-30) Selected the 36-row wave/reconvergence cluster from the Slice 139 census and
  separated its 19 healthy MVP rows from extension-tier advanced wave cases.
- [x] (2026-08-30) Identified scalar masked reductions and prefixes as the largest coherent
  producer family: exact CUDA-specialized final `IRGenericAsm` helpers with one scalar value and
  one `uint4` partition mask.
- [x] (2026-08-30) Added a finite scalar masked-wave recipe descriptor, exact
  spelling/signature resolution, complete capability preflight, and compact generic loop
  emission.
- [x] (2026-08-30) Added focused positive, negative, real-provider, ptxas, and differential
  runtime evidence.
- [x] (2026-08-30) Probed all 36 cluster workloads, promoted 12 newly correct fixtures, and
  regenerated the fixed census/Pareto and representative metrics.
- [x] (2026-08-30) Completed self-review, durable design/capability documentation, final
  provider/compiler/unit builds, all promoted direct lanes, the 410/410 selected prefix, and
  clean formatting/diff validation.

## Surprises and Discoveries

- The census cluster contains 36 rows, but only 19 belong to the bounded MVP denominator. The
  other 17 are explicitly extension-tier advanced wave or quad operations. Both populations are
  useful evidence, but only the 19 healthy MVP rows change the MVP coverage numerator.
- First diagnostics understate the scalar family. Five rows first stop at `_waveMin`, four at
  `_wavePrefixSum`, five at scalar `_waveSum` across signed and unsigned values, and two at
  `_waveProduct`; later scalar helpers in the same fixtures are hidden behind those stops.
- The provider already exposes every primitive needed for a correctness-first lowering:
  `WAVE_READ_LANE_AT`, lane index, shifts, masks, comparisons, scalar combine operations, and
  select. A new provider callback or ABI revision would duplicate expressible compiler logic.
- A first 32-way unrolled implementation was correct at O0 and made 13 cluster workloads correct,
  but libNVVM O3 retained enough live values that `wave-active-product` PTX used 134 b32 SSA
  registers. CUDA 12.9 `ptxas` then failed register allocation. This is concrete evidence that
  the unrolled representation is unusable, not merely a speculative performance concern.
- The compact set-bit loop preserves the same algebra and arbitrary-mask semantics while giving
  libNVVM a small CFG with two phis. The provider validates phi incoming edges against the
  complete function CFG, so emission must terminate source, loop, body, and exit blocks before
  adding the incoming edges.

## Decision Log

- Decision: Implement one exact scalar masked-wave algebra, not one resolver branch per fixture.
  Rationale: `source/slang/hlsl.meta.slang` is the canonical producer of a finite set of reduction
  and prefix spellings. Operation, exclusive/inclusive mode, scalar type, and `uint4` mask shape
  form the semantic key.
  Date/author: 2026-08-30, Codex.
- Decision: Use a compact set-bit loop composed from existing generic builder operations.
  Rationale: The initially planned unrolled scan was differentially correct but generated O3 PTX
  that `ptxas` could not allocate. The loop visits `remaining & -remaining`, obtains the source
  lane with `firstbitlow`, combines its shuffled value, and clears that bit. It preserves the same
  arbitrary-mask algebra with bounded live state and no provider-specific callback.
  Date/author: 2026-08-30, Codex.
- Decision: Keep vector/matrix/array `*Multiple`, active-mask materialization, shuffle/rotate, and
  non-scalar advanced wave forms outside this slice.
  Rationale: They need aggregate decomposition or one genuinely new active-mask primitive. The
  scalar algebra is independently measurable and is the largest reusable subfamily.
  Date/author: 2026-08-30, Codex.
- Decision: Do not revise provider ABI revision 29.
  Rationale: Every emitted operation is already expressible through the generic construction and
  typed value-operation interfaces. Revisit only if a canonical operation cannot be represented
  correctly by that closure.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The bounded scalar algebra makes 12 workloads correct at both modes: 11 healthy MVP references
and one extension workload. No previously correct identity regresses. Direct O0/O3 correctness is
330/335 out of the fixed 452 workloads; healthy-MVP O0/O3/both correctness is
328/332/328 out of 427 (76.8%/77.8%/76.8%). The wave/reconvergence cluster falls from 36 to 24
total rows and from 19 to eight healthy-MVP rows.

The compact loop is the important representation result. The rejected unrolled prototype proved
that semantic correctness alone was insufficient: its O3 PTX exhausted `ptxas` registers. The
final representation passes O0/O3 differential runtime and ptxas, and reuses ABI revision 29.
Remaining wave rows separate cleanly into aggregate `*Multiple`/out-parameter transport,
active-mask materialization, and advanced rotate operations. The overall healthy-MVP Pareto is now
helper ABI/type contract (16), aggregate/pointer/layout transport (14), ordinary numeric/bit
operation (11), and two eight-row wave/atomic families.

## Context and Current Pipeline

Consider `WaveMultiSum(value, mask)` from `source/slang/hlsl.meta.slang`. CUDA specialization emits
the exact final helper body:

    __intrinsic_asm "_waveSum($1.x, $0)";

`StmtLoweringVisitor::visitIntrinsicAsmStmt` creates an `IRGenericAsm`. Linking and CUDA
specialization preserve a one-block helper with scalar result, scalar parameter zero, and
`vector<uint,4>` parameter one. Before this slice, `_validateNVVMFunction` checked the established
catalog, scalar recipe, and compound-wave resolvers before producing the deterministic diagnostic
recorded in `census.slice-139.tsv`.

The new compiler-owned resolver will accept only canonical final helpers, exact assembly, and the
complete signature. It extracts mask lane zero and visits exactly its set bits in a compact CFG
loop. Each iteration uses masked `WAVE_READ_LANE_AT`, combines the selected value through the typed
operation identified by the recipe, and clears the visited bit. Prefix recipes additionally
compare each source lane with the current lane and start from the operation's exact identity.
Capability collection records the complete operation closure before provider discovery or module
mutation.

The canonical shape is valid and intentionally produced by target specialization. The direct
emitter owns it because this boundary translates finalized target-specific semantics into the
provider's generic typed IR operations. No malformed upstream representation is being repaired.

## Scope and Non-Goals

In scope:

- exact scalar `_wave{Sum,Product,Min,Max,And,Or,Xor}` masked reductions;
- exact scalar exclusive/inclusive prefix variants produced by `hlsl.meta.slang`;
- selected signed/unsigned 32-bit integers and Float32 where the combine operation is already in
  the typed value algebra;
- arbitrary canonical 32-bit partition masks carried in `uint4.x`;
- deterministic rejection of adjacent spellings, signatures, types, and non-final helper bodies;
- measurement and promotion across the fixed census.

Out of scope:

- vector, matrix, fixed-array, or `*Multiple` helpers;
- active-mask construction, shuffle/rotate, quad, match, advanced wave, or cooperative operations;
- provider callbacks, ABI revision 30, fixture-name checks, substring parsing, syntax
  reconstruction, compatibility fallbacks, or source-prelude changes;
- performance-specific butterfly or tree reduction; the measured compact loop is the bounded
  correctness representation for this slice.

## Architecture and Invariants

The recipe descriptor owns a finite semantic operation, reduction/prefix mode, inclusive/exclusive
mode, exact scalar type, exact parameters, identity bit pattern, and diagnostic name. Exact spelling
tables may select the descriptor, but emission never interprets substrings or placeholders.

Every recipe has the same structural contract:

- the helper is final and canonical according to `_isCanonicalNVVMGenericAsmValueHelper`;
- result type equals value parameter zero;
- parameter one is exactly `vector<uint,4>`;
- the scalar type and combine operation form a supported typed value-operation descriptor;
- lane and mask predicates are Boolean, source lanes are signed i32 constants, and the partition
  mask is unsigned i32;
- preflight and emission use the same descriptor construction functions;
- the provider sees only typed values and operations, never GenericAsm text.

For reductions, the result starts at the operation identity and includes every set bit in the
partition mask. For exclusive prefixes it includes set source lanes strictly below the current
lane. Inclusive prefixes use lanes below or equal to the current lane. These definitions make
arbitrary partition masks explicit and avoid depending on one prelude fast path.

## Interfaces and Dependencies

No public interface or provider ABI change is planned. `source/slang/slang-emit-nvvm.cpp` gains the
internal recipe resolver, requirements collector, constant construction, and emitter. Existing
`NVVMIRBuilder` generic operations remain the sole provider interface.

Focused fake-provider tests live in `tools/slang-unit-test/unit-test-nvvm-emitter.cpp` with source
fixtures in `tools/slang-unit-test/unit-test-nvvm-support.h`. Real-provider evidence uses the
existing integration helpers. Census scripts and the fixed workload list remain under
`issue-nvvm-backend/` and `build/nvvm-census/`.

## Milestones

1. Add exact recipe classification and fake-provider graph tests for one reduction and one prefix,
   plus adjacent invalid spelling/signature/type/body negatives.
2. Generalize the same descriptor to the complete bounded scalar family and emit the compact
   set-bit loop through existing operations.
3. Build the provider/host, run focused compiler/emitter tests, and prove real PTX, ptxas, and GPU
   differential correctness for divergent and partitioned masks.
4. Run the 36-row cluster probe followed by the fixed 452-workload O0/O3 census. Promote every
   newly correct file and record later root causes rather than widening speculatively.
5. Regenerate Pareto and representative metrics, update durable design/capability documents,
   self-review every new helper/special case, format, run the full selected regression, and commit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox. On this Windows machine use:

    cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release
    cmake.exe --build build --config Release --target slang-unit-test
    $env:SLANG_NVVM_BUILDER_PATH =
      'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
    .\build\Release\bin\slang-test.exe slang-unit-test-tool/nvvm

Acceptance requires focused positive/negative fake-provider tests; real-provider IR and ptxas
evidence; O0/O3 differential runtime coverage including divergent masks; every promoted fixture;
the fixed census with zero previously correct regression; representative metrics/SM70-SM80-SM90
ptxas checks; clean formatting; `git.exe diff --check`; and the complete selected prefix.

## Failure and Recovery

The change is additive. If an exact spelling or scalar type fails differential runtime, keep it
outside the resolver and record its exact producer and mismatch; do not insert a fixture check or
fallback. If the scan is semantically wrong under divergence, compare its mask/lane predicates
against the CUDA prelude contract and fix the recipe invariant. If an operation is unsupported by
the provider, preflight must fail before module creation and name the missing typed operation.

Rerunning builds, tests, census, and metrics is safe. The existing direct path remains available by
removing the new resolver/recipe branch without changing provider binaries or ABI negotiation.

## Self-Review

Inventory every new helper and exact spelling row. For each, record the canonical producer,
complete signature, why this target-specific emitter owns it, and the test that fails without it.
Reject any row that exists only for one fixture or any emission branch that reconstructs source
syntax. Perform a revert drill on the shared resolver or emitter and prove that the focused family
returns to deterministic GenericAsm preflight rather than passing through another fallback.

## Artifacts and Hand-Off

Keep local probe output under `build/nvvm-census/slice140-*`. Retain a fixed Slice 140 census TSV,
cluster JSON, and five-part report under `issue-nvvm-backend/`. Distill stable scalar-wave recipe
architecture into `docs/design/nvvm-backend.md` and exact measured capability status into
`docs/design/nvvm-backend-capability-ledger.md`. The next slice should use the new Pareto to choose
between aggregate `*Multiple` decomposition and the separate active-mask/shuffle family.
