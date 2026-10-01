# Validate and refresh NVVM results

This is a results-only workflow. It does not restart the development loop. Run from the repository
root on native Linux, with matching optimized compiler, provider and test tools. Read STATUS and
WORKFLOW first. Rebuild through `slang-build` after source changes; never benchmark stale binaries.
The maintained entry point is `python3 issue-nvvm-backend/nvvm-results.py --help`.
When configuring after a revision change, clear cached `SLANG_VERSION_FULL` and
`SLANG_VERSION_NUMERIC` (`cmake --preset default -U SLANG_VERSION_FULL -U SLANG_VERSION_NUMERIC`
plus the selected build options on this native Linux host). Verify `slangc -version` identifies
the compiler source revision; source and binary hashes remain the acceptance identity.

The expanded discovery corpus also requires the standard numerics modules from the same build.
Ensure the existing `slang-numerics-modules` target is included when building through `slang-build`;
it produces `slang/numerics.slang-module` and the three serialized modules under
`slang/numerics/` in the configured standard-module directory. Do not copy serialized modules from
a different compiler build. For a results-only refresh, verify these artifacts against the accepted
ledger along with the compiler/core module. Missing modules are a packaging failure, not evidence
that the shader or backend is unsupported. Preserve such failed attempts before restoring the
matching layout or rebuilding and requalifying it.

## Evidence and outputs

Every harness command requires a **new** `--output` directory and refuses to overwrite old evidence.
Use a unique ignored root, for example `build/nvvm-results/2026-09-28-refresh1`. Failed attempts stay
there. Measurements retain every command, exit, timeout, log, phase, PTX/cubin and content hash.
Provenance records revision, working diff, submodules, compiler/provider/cache/toolkit bytes, sources,
inputs and device query. Review the actual loaded libraries as well when changing environments.
Do not publish the full environment/raw trees inadvertently; only compact reviewed results belong
in the repository. Commands default to SM80 and matching CUDA12.9 on this host.

```bash
export NVVM_RESULTS=build/nvvm-results/2026-09-28-refresh1
export NVVM_BASELINE=issue-nvvm-backend/accepted-baseline.json
export CUDA_PATH=/usr/local/cuda-12.9
export CUDA_HOME="$CUDA_PATH"
export LIBNVVM_HOME="$CUDA_PATH"
export SLANG_NVVM_TEST_ARCH=80
export SLANG_NVVM_BUILDER_PATH="$PWD/build/RelWithDebInfo/bin/libslang-llvm-nvvm.so"
export LD_LIBRARY_PATH="$PWD/build/RelWithDebInfo/lib:$PWD/build/RelWithDebInfo/bin:$CUDA_PATH/nvvm/lib64:$CUDA_PATH/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
```

The harness selects that toolkit's NVVM/NVRTC libraries and exact provider. A command failure returns
nonzero and keeps partial evidence. Census exit2 can describe established gaps; exact structured
comparison decides preservation. Running processes are killed as a group after their timeout.

## Development corpus tiers

The tier selector reuses the census and discovery runners. Its initial scope is eligible authored
CUDA `COMPARE_COMPUTE` runtime cases: source, test ordinal and backend/optimization identify each
configuration. Compile-only/PTX tests, rejection units and custom memory/material/OptiX hosts retain
their focused commands. Smoke is a subset of working, not a second source tree.

```bash
python3 issue-nvvm-backend/nvvm-results.py corpus --tier smoke \
  --output build/nvvm-results/smoke-1
python3 issue-nvvm-backend/nvvm-results.py corpus --tier working \
  --output build/nvvm-results/working-1
python3 issue-nvvm-backend/nvvm-results.py corpus --tier exploratory \
  --modes nvvm-o3 --limit 12 --output build/nvvm-results/explore-1
```

Add `--list-only` to inspect selection without invoking compiler or GPU tools. Every output directory
must be new. Run smoke after implementation iterations, working every three to five iterations or
after broad changes, and bounded exploratory batches when selecting work. A completed exploratory
run can contain failures; inspect `selection.json` outcomes rather than interpreting command success
as shader support. No run promotes or demotes configurations automatically.

`corpus-tiers.json` owns the current smoke choices, exploratory priorities and exact focused
admission mappings. The accepted full baseline still owns its historical outcomes; focused evidence
owns later qualifications. Review the authored directive, adapted runner arguments, mode and oracle
before admitting a focused cell. A source hash and an unrelated passing test are insufficient.
Sidecar oracle hashes retain their accepted provenance. Changed inputs remain working expectations
but require review; a working failure remains a regression. Preserve these histories when updating
the compact metadata. The selector's CPU contracts are `test-nvvm-corpus-tiers.py` together with the
existing census, discovery and results contracts.

### OptiX raygen gate

Build matching compiler, provider and unit tools with eight jobs, then run the permanent O0/O3 PTX
fixture and the focused SDK/stage/runtime checks:

```bash
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 1 -disable-retries \
  tests/cuda/nvvm-optix-raygen.slang \
  slang-unit-test-tool/nvvmIRBuilderOptixPrimitivesKeepExactSignatures.internal \
  slang-unit-test-tool/nvvmSlangOptixPrimitivesRejectComputeBeforeEmission.internal \
  slang-unit-test-tool/nvvmOptixRaygenBindings.internal
```

The runtime unit requires actual execution without skips on an OptiX host. It checks module,
program-group and pipeline creation, reflected launch/SBT ABI, two changed launches per optimization
mode, complete output and guards. It uses the existing dynamically loaded OptiX unit infrastructure;
no SDK/runtime installation is part of this gate. Run the existing CUDA
`tests/pipeline/ray-tracing/raygen.slang` control and the compute smoke selection when changing shared
entry lowering. The separate static-build selector
`nvvmOptixSbtPlansKeepStageAndTypeBoundaries` checks SBT layout, stage admission and load flags before
provider mutation. These checks qualify raygen only, not triangle tracing or payload transport.

### OptiX triangle gate

Build `render-test` alongside the compiler/provider/unit targets. The triangle fixture runs UInt and
Float4 payloads at NVVM O0/O3 with selected NVRTC O3 controls; every case checks all output words and
untouched storage against independent expectations. It uses the existing real triangle scene and
conventional `missMain`/`closestHitMain` entries in the render-test pipeline.

```bash
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 1 -disable-retries \
  tests/pipeline/ray-tracing/nvvm-optix-triangle.slang \
  slang-unit-test-tool/nvvmIRBuilderOptixPayloadRegistersRejectWithoutMutation.internal \
  slang-unit-test-tool/nvvmIRBuilderOptixTracePreservesTupleAndRejectsWithoutMutation.internal
```

Require actual execution without skips. Retain the raygen gate and compute smoke after shared
entry/payload changes. Static selectors `nvvmOptixTracePlansKeepPayloadAndStageBoundaries` and
`nvvmAccelerationHandlesKeepOpaqueRoles` cover retained payload types, stage restrictions, absent
provider support before mutation, and opaque handle roles. These tests do not qualify arbitrary
OptiX stages, recursive rays or pointer payloads.

### OptiX material and ray-state gate

```bash
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 1 -disable-retries \
  tests/pipeline/ray-tracing/nvvm-optix-material.slang \
  slang-unit-test-tool/nvvmIRBuilderOptixPrimitivesKeepExactSignatures.internal \
  slang-unit-test-tool/nvvmSlangOptixRayStateRejectsOtherStagesBeforeEmission.internal
```

The fixture checks 202 words per mode, including all eight world-ray observations in both callbacks,
texture channels, material value/PDF lanes and guards. Material values use the retained dielectric
absolute/relative budget; CUDA output is comparison evidence. The grouped negative unit calls all
eight raw SDK helpers from compute and raygen to prove backend rejection before provider creation.
Use compute smoke and raygen binding controls after this named-query batch. Unchanged triangle
storage/type/IR contracts retain their earlier static evidence; no new static rebuild is needed.

## Optional slang-rhi CUDA suite

This is on-demand application validation. The prepared sibling checkout needs the test harness's
`--cuda-compiler=nvvm|nvrtc` selector. Its existing `build-all` uses a downloaded compiler; keep that
build intact. From the Slang repository root, configure a separate build against the local headers
and package root (the directory containing `lib/` and `bin/`):

```bash
export RHI_SOURCE="$(realpath ../slang-rhi)"
export RHI_BUILD="$PWD/build/nvvm-rhi-cuda/rhi-build"
cmake -S "$RHI_SOURCE" -B "$RHI_BUILD" -G 'Ninja Multi-Config' \
  -DSLANG_RHI_FETCH_SLANG=OFF \
  -DSLANG_RHI_SLANG_INCLUDE_DIR="$PWD/include" \
  -DSLANG_RHI_SLANG_BINARY_DIR="$PWD/build/RelWithDebInfo" \
  -DSLANG_RHI_BUILD_TESTS=ON -DSLANG_RHI_BUILD_EXAMPLES=OFF \
  -DSLANG_RHI_BUILD_TESTS_WITH_GLFW=OFF -DSLANG_RHI_ENABLE_CUDA=ON \
  -DSLANG_RHI_ENABLE_OPTIX=ON -DSLANG_RHI_ENABLE_CPU=OFF \
  -DSLANG_RHI_ENABLE_VULKAN=OFF -DSLANG_RHI_ENABLE_WGPU=OFF \
  -DFETCHCONTENT_SOURCE_DIR_OPTIX_8_0="$RHI_SOURCE/build-all/_deps/optix_8_0-src" \
  -DFETCHCONTENT_SOURCE_DIR_OPTIX_8_1="$RHI_SOURCE/build-all/_deps/optix_8_1-src" \
  -DFETCHCONTENT_SOURCE_DIR_OPTIX_9_0="$RHI_SOURCE/build-all/_deps/optix_9_0-src"
cmake --build "$RHI_BUILD" --config RelWithDebInfo --parallel 8 --target slang-rhi-tests
```

These dependency overrides reuse the prepared local OptiX headers; they do not install a toolkit
or change the existing RHI build. RHI derives target capabilities from its CUDA device;
`SLANG_NVVM_TEST_ARCH` controls Slang test tools and does not set the RHI shader target. Use the matching CUDA/provider/library environment from
[Evidence and outputs](#evidence-and-outputs). Rebuild Slang first after compiler changes, then
refresh this RHI target when its headers or sources change. Check `ldd` on the test executable and
record both repository revisions/patches and actual compiler/provider/test-binary hashes.

Qualify the setup with a bounded selection in separate processes:

```bash
"$RHI_BUILD/RelWithDebInfo/slang-rhi-tests" --select-devices=cuda --require-devices=cuda \
  --cuda-compiler=nvvm --test-case=compute-trivial.cuda
"$RHI_BUILD/RelWithDebInfo/slang-rhi-tests" --select-devices=cuda --require-devices=cuda \
  --cuda-compiler=nvrtc --test-case=compute-trivial.cuda
```

The focused `ray-tracing-intrinsics-hit-identities.cuda` case uses two instances with distinct custom
IDs and three primitives each. It runs O0 and O3 internally through the shared compiler selector,
checking six hits, one miss and all 44 output words, including both triangle faces and alternating
ray flags. Pair it with
`ray-tracing-triangle-intersection.cuda` when changing typed hit queries or conventional globals:

```bash
"$RHI_BUILD/RelWithDebInfo/slang-rhi-tests" --select-devices=cuda --require-devices=cuda \
  --cuda-compiler=nvvm \
  --test-case=ray-tracing-intrinsics-hit-identities.cuda,ray-tracing-triangle-intersection.cuda
```

The focused AnyHit controls are `ray-tracing-intrinsics-payload-termination.cuda`,
`ray-tracing-intrinsics-object-ray-origin.cuda` and
`ray-tracing-intrinsics-object-ray-direction.cuda`. Each runs O0/O3 internally. The payload test
checks nine direct/nested/conditional termination modes and normal-return controls; the object
cases use exact independent affine expectations without normalizing the direction. Existing
`ray-tracing-intrinsics-accept-hit-and-end-search.cuda` and
`ray-tracing-intrinsics-ignore-hit.cuda` are small supplemental controls. Select these names with
the same compiler/device options above; they do not require a full-suite run.

Only when the broader suite is requested, run the same command with `--test-case='*.cuda'`.
Keep NVVM and NVRTC logs separate, retain initial failures, and inspect executed/failed/skipped
counts and individual diagnostics. A zero-test filter or skipped test is not runtime qualification.
Use focused retries for investigated failures; do not rerun the entire suite after each fix.

The compiler selector covers CUDA availability and the shared `createTestingDevice` path.
Tests constructing custom devices or Slang sessions can bypass it, and RHI's internal CUDA clear
kernels still use NVRTC directly. Therefore a CUDA suite run is not proof that every shader used
NVVM. The standalone `nvrtc` test is also direct NVRTC and is outside the `*.cuda` filter. Keep
these application results separate from Slang's accepted full baseline and tier admissions.

## Correctness baseline and comparison

For an authorized migration after a host/driver change with unchanged verified compiler/toolkit
bytes, follow WORKFLOW's consolidated sequence: run the existing runtime and toolkit validators
first, capture old migration controls on the new environment, then run the full checkpoint and
remaining full-acceptance gates on the final compiler. Preserve the historical accepted baseline
until exact comparison and review succeed. Do not duplicate focused tests already represented by
the final full run; retain their exact identity-to-result mapping.

After a master/toolchain merge, first run the small GPU gate and full corpus wrapper:

```bash
python3 issue-nvvm-backend/nvvm-results.py checkpoint \
  --baseline "$NVVM_BASELINE" \
  --slangc build/RelWithDebInfo/bin/slangc --build-label RelWithDebInfo \
  --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
  --jobs 4 --output "$NVVM_RESULTS/checkpoint"
```

This runs the established four-fixture runtime validator, physical surface suite, frozen1356 inventory,
discovery manifest and every material support cell sequentially. It writes `checkpoint.json`,
`comparison.json` and `outcomes.json`. Full compiler acceptance **also** needs these gates, sequentially with no benchmark:

```bash
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries slang-unit-test-tool/
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
  tests/language-feature/generics tests/language-feature/overload \
  tests/language-feature/operator-overload tests/diagnostics tests/serialization
python3 extras/validate-nvvm-toolkit.py --slangc build/RelWithDebInfo/bin/slangc \
  --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
  --expected-toolkit 12.9 --architectures 80 --output "$NVVM_RESULTS/toolkit"
python3 issue-nvvm-backend/test-run-compute-census.py
python3 issue-nvvm-backend/test-run-compute-discovery.py
python3 issue-nvvm-backend/test-run-complex-corpus.py
python3 issue-nvvm-backend/test-nvvm-results.py
python3 issue-nvvm-backend/test-nvvm-surface-results.py
python3 issue-nvvm-backend/test-nvvm-material-runtime.py
python3 issue-nvvm-backend/test-nvvm-corpus-runtime.py
```

Capture each gate's command, exit and log under the results root; use `timeout --kill-after=30s 30m`
for long gates. Compare unit/semantic identities and statuses with the last accepted ledger, including
skips and upstream additions; totals alone do not prove preservation. Toolkit and smoke validators
require real completed cells. Keep expected unresolved/runtime failure histories visible.

Changes to type representations, local record-array roles or address provenance also require the
direct static units. Follow the build skill to configure an isolated `SLANG_LIB_TYPE=STATIC` test
build and build `slang-static-unit-test`; the ordinary shared test plugin cannot call these internal
compiler APIs.
Reuse the existing provider, keep the accepted shared layout separate, and record both source and
configuration identities. On this native host the isolated test configuration disables unused DXIL
and `slang-llvm` fetching and uses already available dependency sources. Do not add production export
hooks or link a second compiler into the shared test plugin merely to expose these calls.

```bash
SLANG_NVVM_BUILDER_PATH="$PWD/build/RelWithDebInfo/bin" \
  "$NVVM_STATIC_BUILD/RelWithDebInfo/bin/slang-static-unit-test" nvvmLocalRecordArray
```

Require both named tests to execute and pass without skips: cache orders/role refusals and exact
local allocation and other pointer-producer address plans. Pin the static executable/provider/configuration and retain
its output separately from shared native-unit identities. The shared units and permanent three-mode
fixtures remain required; direct IR tests do not replace GPU execution.

For Half helper-boundary changes, also run the three direct type-lowering units in
`unit-test-nvvm-type-lowering.cpp` (selectors `nvvmHalf` and `nvvmScalarHalf`). Require actual execution
without skips of both cache-order units and the exact classifier unit. The scalar test preserves an
existing invariant; it is not evidence of a reproduced scalar cache defect. Run the shared
`nvvmSlangFloat16ValuesUseGenericTypedPipeline`,
`nvvmSlangHalfVectorsCrossAllFourHelperBoundaries`, and
`nvvmSlangHalfVectorBoundaryCapabilitiesPreflightEveryWidth` units, followed by the three-mode
`tests/cuda/nvvm-half-vector-helper-` fixtures with retries disabled.

Capture actual provider LLVM using `-dump-intermediates -dump-intermediate-prefix UNIQUE_PREFIX`
in addition to `-dump-ir`; inventory and hash the produced LLVM and PTX rather than relying on dump
ordinals. Inspect the four conversions, retained calls and native body operations. For exported
boundaries, freeze independently authored PTX caller declarations before repair, compare symbol,
visibility, size, alignment and semantic lane offsets for each width, then execute that same caller
against before-O0 and repaired O0/O3 helpers. Check complete outputs, completion and guards; ignore
unspecified padding. Preserve wrong-output before cells. Same-module Slang calls do not substitute
for the external caller, and this gate does not establish CUDA-prelude binary interoperability.

For surface-format qualification, pair shader checks with independent host readback of the actual
CUDA array. Freeze logical texels and their physical byte encoding before execution, query the created
array's channel format/count/extents, and copy the complete allocation back without shader texture
loads. Check untouched neighboring texels as well as written channels. Run the same kernels with a
matching native-width allocation as a control; shader read/write agreement alone can share a wrong
byte-X scale. The historical binding investigation in the `surface-physical-format` focused record
pins a driver, frozen signed8 and signed32 oracles, source and ten launch results. These raw research files stay under ignored `build/`;
a production format expansion needs permanent reproducible host-readback coverage.

For CPU-only comparison of frozen/discovery outcomes against a durable old compact baseline, even
without the old raw directory, use `compare`. This command does not validate physical surfaces;
use the full `checkpoint` for recurring surface acceptance:

```bash
python3 issue-nvvm-backend/nvvm-results.py compare \
  --baseline "$NVVM_BASELINE" \
  --frozen "$NVVM_RESULTS/checkpoint/frozen/results.json" \
  --discovery "$NVVM_RESULTS/checkpoint/discovery/results.json" \
  --output "$NVVM_RESULTS/comparison"
```

The baseline must be explicitly `accepted-full`. Missing/duplicate cells, changed five-field outcomes,
incomplete mode inventories and false passes are rejected. Additions require `--allow-additions`
and all three correct modes. Comparisons retain unresolved/resolved histories. Input hashes can
legitimately change upstream: checkpoint writes each old/new hash delta and stops at `review-required`.
Do not overwrite old evidence or feed the rejected outcomes back as an accepted baseline.

After all gates and exact deltas are reviewed, replace `accepted-baseline.json` with the complete current record from
`outcomes.json`, set `status: accepted-full`, attach gate evidence, reviewed input/outcome transitions,
and update the failure histories. Keep the original comparison under the ignored results root and identify the previous baseline by Git revision, path and hash. A manual
status edit without that review is not acceptance. The compact schema produced by checkpoint already supplies all required fields:

```json
{
  "schema": 1,
  "status": "accepted-full",
  "baseline": { "path": "previous accepted JSON", "sha256": "..." },
  "provenance": {
    "revision": "tested revision",
    "artifact_sha256": { "absolute path": "sha256" }
  },
  "runtime_input_sha256": { "tests/path.slang": "sha256" },
  "corpora": {
    "frozen": { "fresh_cell_outcomes": ["all id/mode/five-field rows"] },
    "discovery": { "fresh_cell_outcomes": ["all id/mode/five-field rows"] }
  },
  "unresolved_failures": [],
  "resolved_failure_history": [],
  "acceptance": {
    "gate_evidence": [],
    "reviewed_transitions": [],
    "reviewed_input_deltas": []
  }
}
```

The sample shows field structure only; preserve actual complete rows and histories. Copy the complete
checkpoint outcomes, add reviewed gate/transition evidence, and change status only after acceptance.
The harness deliberately has no automatic promotion command: a matching count is not a review.
Update the current record in place; Git retains the old accepted snapshot. Do not add another
slice-numbered validation file. Retain exact cells and failure transitions, not nested old baselines.
If a run has an infrastructure failure, retain the failed comparison. A supplemental closure needs
an explicit, predeclared scope and review: preserve every failed attempt, require all declared rounds
to pass, identify each substituted cell and its source evidence, and distinguish original counts from
composite accepted counts. Serial closure never proves concurrent reliability. Historical NVRTC PCH incidents and their later
per-owner-directory resolution remain in the accepted evidence; do not reintroduce an already resolved
limitation as current. Do not use retries to hide shader-output regressions or failed timing samples.

Use the reviewed current `NVVM_BASELINE` identified by STATUS before quality measurement. It must
retain `runtime_input_sha256`,
`provenance.artifact_sha256`, and per-corpus `fresh_cell_outcomes` for the next refresh.

### Rounding signature and numerical contracts

Run the read-only oracle check before the focused shader tests:

```bash
python3 extras/test-generators/check-nvvm-round-oracles.py --check
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
  tests/cuda/nvvm-round-
```

The prefix includes three numerical fixtures and the removed semantic-tag source control. Each
numerical fixture has NVRTC O3, NVVM O0 and NVVM O3 directives. Require all declared identities and
no skips; preserve the compiler/provider, source and expected-buffer hashes. Float32/64 use exact
ties-away expectations, while Half checks both ties-away and ties-even arrays unconditionally with
backend-specific FileCheck masks. Signed zeros/infinities compare bits and NaNs compare class.
Scalar, noinline helper, vector2/3/4 and matrix2x2 outputs retain guards and per-lane completion.
The checker uses only integer IEEE decoding and Fraction arithmetic; shader or vendor output must
not regenerate expected values. These focused contracts do not change frozen corpus membership.

For the combined ceil/floor/trunc contracts, run:

```bash
python3 extras/test-generators/check-nvvm-directed-rounding-oracles.py --check
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
  tests/cuda/nvvm-directed-rounding-
```

Require all nine numerical mode cells and three removed-tag diagnostics. Each width uses 64 live
IEEE inputs and 45 scalar/helper/vector2/3/4/matrix2x2 observations per lane. Six mask bits per
operation, completion words and end guards must match exactly. Expected constants come from rational
arithmetic and are checked independently by integer IEEE bit operations. Preserve signed zeros and
infinities exactly; NaNs promise classification only. Keep the separate round fixtures unchanged.

### Native Half fused multiply-add contract

`tests/cuda/nvvm-half-fma.slang` defines the NVVM Half contract as
`RN16(exact(a*b+c))`: one nearest-even rounding, no FTZ and no saturation. Explicit `fma` remains
fused in precise floating mode; this does not authorize contracting a separate `a*b+c` expression.
Scalar Half `ceil`, `floor`, `trunc` and `fma` use the genuine LLVM registry intrinsics at f16 width.
Float32/64 libdevice paths and the physical i16 Half helper ABI are unchanged. The qualified
libNVVM 12.9 verifier rejects `llvm.trunc.f16`; after registry, physical operand and collision checks,
the provider emits pure `cvt.rzi.f16.f16` with 16-bit constraints and no unsupported declaration.
Mechanical Half→i16→Half bitcasts satisfy the verifier’s assembly operand rules without changing
arithmetic precision.
Ceil/floor/fma keep their genuine LLVM declarations. Whole-module verification remains enabled.
The native Half instructions require SM53 or newer; this batch is qualified at SM80. The backend
does not currently impose a separate Half architecture floor, and SM50/52 compatibility is unqualified.

The fixture has four cells: CUDA O3 comparison, NVVM O0, NVVM O3 and NVVM O3 precise. Seventeen
live input triples each exercise scalar, noinline helper and four heterogeneous vector lanes.
Require all six error bits clear, exact scalar bits (NaNs normalized only for class comparison),
completion words 100–116 and endpoint guards: 53 uint words total. Signed zeros and infinities are
exact; no NaN sign/payload is promised. Finite expectations were derived with exact rational
arithmetic and nearest-even binary16 selection, independently of compiler or vendor output:

| a    | b    | c    | Expected Half bits | Distinction                                |
| ---- | ---- | ---- | ------------------ | ------------------------------------------ |
| 3c01 | 3e00 | 8001 | 3e01               | Float32 intermediate instead produces 3e02 |
| 3c01 | 3c01 | bc02 | 0010               | Fused cancellation residual                |
| 7bff | 4000 | fbff | 7bff               | Recoverable product overflow               |
| 0001 | 3800 | 0000 | 0000               | Underflow tie to even zero                 |
| 0003 | 3800 | 0000 | 0002               | Underflow tie to even second subnormal     |
| 8001 | 3800 | 8000 | 8000               | Negative underflow tie                     |
| 03ff | 3c00 | 0001 | 0400               | Subnormal/normal boundary                  |
| 7bff | 3c00 | 4c00 | 7c00               | Overflow tie to infinity                   |

The remaining cases independently check opposite-sign zeros (positive zero), two negative zeros
(negative zero), exact cancellation (positive zero), signed infinity plus a finite term, infinity
times zero, opposite infinities, and quiet/signaling NaNs. Signed exact-zero rules are not inferred
from a signless rational zero. Keep the existing 64-input directed-rounding fixture and its
integer-bit checker unchanged; the direct instructions preserve that contract.

The changed `3e02` to `3e01` result is an intentional correction, not byte-for-byte baseline
preservation. CUDA agreement is comparison evidence. [GLSL.std.450 Fma](https://registry.khronos.org/SPIR-V/specs/unified1/GLSL.std.450.html)
and the legacy [Vulkan GLSL Fma precision contract](https://docs.vulkan.org/spec/latest/appendices/spirvenv.html)
do not provide a universal exact binary16 FMA oracle; Vulkan's distinct correctly rounded
`OpFmaKHR` requires its own features. Slang's Half HLSL path uses
[mad](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/mad), whose Direct3D contract
permits fused or separate evaluation, and historical HLSL half need not be native 16-bit arithmetic.
Cross-backend signed-zero/subnormal/NaN comparisons also depend on enabled floating-point controls.
The selected non-FTZ [PTX Half FMA](https://docs.nvidia.com/cuda/archive/12.9.1/parallel-thread-execution/index.html#half-precision-floating-point-instructions)
implements the explicit NVVM contract. Keep existing Slang target-dependent `round` ties-away
unchanged; CUDA Half and HLSL ties-even are not an implicit policy change. Approximate Half
exp2/tanh have separate accuracy/architecture contracts and remain deferred, without relaxing the
default transcendental policies.

### Square-root signature and numerical contracts

```bash
python3 extras/test-generators/check-nvvm-sqrt-oracles.py --check
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
  tests/cuda/nvvm-sqrt-
```

Require nine numerical mode cells and the removed-tag diagnostic, with no skips. Each width has
64 live IEEE inputs and 15 scalar/noinline helper/vector2/3/4/matrix2x2 observations per lane.
Float32/64 require 130 output words; Half requires 195, including an explicit policy marker and
raw scalar bits. NVVM Half uses exact Float32 sqrt followed by Half rounding; CUDA Half retains
its approximate Float32-root bound before narrowing. The independent checker computes both with
integer arithmetic and integer square root. Signed zeros/infinities compare bits; NaNs promise
classification. Preserve every actual buffer before migration and compare all nine buffers after
migration. Any observed change requires review, even if a CUDA approximation interval permits it;
never regenerate expectations from compiler or vendor output. Keep round and directed-rounding
fixtures unchanged. These focused contracts do not change frozen corpus membership.

### Fraction composition and numerical contracts

```bash
python3 extras/test-generators/check-nvvm-frac-oracles.py --check
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
  tests/cuda/nvvm-frac-
```

Require nine numerical mode cells and one removed-tag diagnostic, with no skips. Each fixture
uses 64 distinct live IEEE inputs and 26 observations per lane: scalar/noinline/vector2/3/4/matrix2x2
`frac`, plus scalar/noinline/vector2/3/4 `fract`. The public alias has no matrix overload. The exact
integer/rational reference rounds `x - floor(x)` to the result width; every finite Half residual is
exactly Float32, so both existing Half evaluation paths have the same finite result. Tiny negative
inputs may round to positive one. Finite results, including zero sign, compare exact bits; either
infinity and NaN inputs require NaN classification.

Each output has 386 uint words: leading guard, 64 records of
`errors, fracLow, fracHigh, fractLow, fractHigh, 59000 + lane`, and trailing guard. Require all eleven
shape-error bits clear, every completion word, exact output size and unchanged guards. Half/Float32
high words are zero; Float64 raw values use low/high pairs. Preserve all nine actual buffers before
migration and compare every byte afterward, including observable NaN payloads. Any observed change
requires review even when the mathematical contract promises only NaN classification. Never derive
expected values from compiler or vendor output. Keep the existing rounding/sqrt fixtures unchanged;
these focused contracts do not change frozen corpus membership.

### Reciprocal-square-root numerical contracts

```bash
python3 extras/test-generators/check-nvvm-rsqrt-oracles.py --check
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
  tests/cuda/nvvm-rsqrt-
```

Require nine numerical cells and one removed-tag diagnostic, with no skips. Each fixture uses 64
unique IEEE inputs and 15 scalar/noinline/vector2/3/4/matrix2x2 observations per lane. Float32/64
outputs have 258 uint words: leading guard, 64 records of `errors, low, high, 65000 + lane`, and
trailing guard. Half adds a policy marker after the leading guard for 259 words: 65039 for NVVM
or 1209 for CUDA. The marker identifies the policy, not the current module version. Require every
completion, zero shape-error masks, exact guards/length and host-checked paired scalar admission;
Half/Float32 high words must be zero.

The library policy is the union of a radius N times the exact same-width RN-even reference's
larger adjacent spacing and an N-encoding-step neighborhood (Float32 N = 2, Float64 N = 1). This
explicit empirical test convention is neither a vendor-defined ULP metric nor a guaranteed bound.
NVVM Half uses the exact discrete RN16 image of the Float32 union. CUDA Half uses the exact PTX
relative-error set with epsilon 2^-22.9: integer tenth-power brackets must give matching inner/outer
Float32 candidate intervals before discrete RN16 narrowing. Special values check signed infinity,
positive zero or NaN class as appropriate. No observed output defines an oracle.

The standalone checker independently recomputes references, admission tables and fixture contracts,
and runs six negative certificate checks without an ignored manifest. Preserve all nine original
buffers and compare every byte after migration, including raw NaNs. Even an admitted alternative
requires review if its raw bits change. Keep earlier math fixtures and frozen corpus membership
unchanged. Exact version 39 module controls separately qualify retirement and reader40 rejection;
the three static module units cover historical versions31–38 and a dynamic future version, and do
not substitute for those exact version39 controls.

### Exponential numerical contracts

```bash
python3 extras/test-generators/check-nvvm-exp-oracles.py --self-test --check
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
  tests/cuda/nvvm-exp-
```

Require nine numerical cells and one removed-tag diagnostic, with no skips. Half/Float32/Float64
fixtures contain 80 / 62 / 62 unique IEEE inputs and 15 scalar/noinline/vector2/3/4/matrix2x2
observations per lane. Half output has 323 uint words: leading guard, target policy marker,
80 records of `errors, low, high, 55000 + lane`, and trailing guard. Float32/64 use 250 words
and 62 records without a policy marker. Half markers `55040` (NVVM) and `1209` (CUDA) are policy IDs,
not module versions. Require six shape-error bits clear, every completion, exact lengths/guards,
host-checked scalar admission and zero high words for Half/Float32.

Library admission is the explicit test-defined spacing/encoding union, N = 2 for Float32 or
N = 1 for Float64, centered on independently certified same-width RN-even references. Zero uses minsubnormal spacing;
maximum finite uses the conceptual next binade for adjacent spacing; infinity admits only itself
and N finite predecessors by ordered encoding steps. Exact special-input rules override the
approximate set: either signed zero returns one, negative infinity returns positive zero, positive
infinity returns positive infinity, and NaNs require only NaN classification.
These endpoint extensions are not a vendor-defined ULP metric or a universal accuracy guarantee.
The library table is empirical and non-guaranteed; PTX ex2 has its separate source contract.

NVVM Half narrows the admitted Float32 expf set once. CUDA Half preserves the exact encoded-FMA
constant `0x3fb8aa3b` with RN FMA and a negative-zero addend, ex2 input/output FTZ, narrowing and
four Half correction FMAs, including inactive stages. The original-input match/correction pairs
are `0x1f79/0x9400`, `0x25cf/0x9400`, `0xc13b/0x0400` and `0xc1ef/0x0200`.
The checker preserves every discrete image. Half baseline scalar results at
`0x1f79` and `0x25cf` intentionally differ between targets; compare each mode to its own immutable
baseline rather than requiring cross-target identity. Preserve all nine complete buffers byte for
byte before/after migration, including NaN payloads that admission only checks by classification.

The standalone checker reconstructs all 204 inputs/references, both Half policies and exact fixture
contracts without an ignored manifest. It runs 38 synthetic midpoint/endpoint/FTZ/constant/correction
controls; optional `--proposal PATH --negative-controls` audits preparation certificates and 16
mutations. Independent generator proofs and checker proofs use different integer algorithms.
Keep all earlier five oracle suites and 15 fixtures unchanged, including six rsqrt negative controls;
exp adds the sixth suite and three fixtures without changing frozen corpus membership.

### Base-two exponential numerical contracts

```bash
python3 extras/test-generators/check-nvvm-exp-oracles.py --operation exp2 --check --self-test
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
  tests/cuda/nvvm-exp2-
```

Require nine numerical cells and one removed-tag diagnostic, with no skips. Half/Float32/Float64
have 74 / 69 / 69 unique inputs and 15 live scalar/noinline/vector/matrix observations per lane.
Outputs contain 299 / 278 / 278 uint words; each lane records errors, raw low/high and `56000 + lane`.
Half adds policy marker `56042` for NVVM or `1209` for CUDA. Require exact lengths, guards, completions,
zero errors, scalar candidate admission and zero high words for Half/Float32. Compare all nine raw
buffers to their own immutable baseline bytes, including unpromised NaN payloads.

Use the exp endpoint convention and exact special-input rules above with references for `2^x`:
Float32 N = 2 and Float64 N = 1. The generator and independent checker certify integer powers,
including exact underflow halfway ties, or bound ln2 and exponential endpoints for noninteger inputs.
NVVM Half narrows selected Float32 library candidates. CUDA Half applies ex2 input/output FTZ,
then exact `fma.rn.f32(q, 2^-24, q)`, then RN16. The multiplier bits are `0x33800000`; rounding
`1 + 2^-24` first, omitting the FMA or moving it after Half narrowing changes the contract.

The checker reconstructs 212 inputs/references without an ignored manifest. Its 57 synthetic
controls comprise 38 shared exp checks and 19 exp2 checks, including independent FMA encoding rules,
midpoint parity, FTZ and biased-narrowing negatives. Optional `--proposal PATH --negative-controls`
audits preparation certificates and rejects 14 mutations. The default operation remains exp.
Preserve the six earlier suites and 18 fixtures unchanged; exp2 adds the seventh suite and three
fixtures outside frozen corpus membership. Targeted acceptance retains the last full baseline's
identity and records these fresh results separately, following WORKFLOW.

### Logarithm-family numerical contracts

```bash
python3 extras/test-generators/check-nvvm-log-oracles.py --check --self-test --negative-controls \
  --cuda-header "$CUDA_PATH/include/cuda_fp16.hpp"
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
  tests/cuda/nvvm-log- tests/cuda/nvvm-log2- tests/cuda/nvvm-log10-
```

Require 27 numerical cells and three removed-tag diagnostics, with no skips. Log and log2 each
contain 91/62/62 Half/Float32/Float64 inputs; log10 contains 91/62/80. Each input has 15 live
scalar/noinline/vector2/3/4/matrix2x2 observations. Half outputs have 367 words, Float32 and ordinary
Float64 outputs have 250, and double log10 has 322. Check all shape-error bits, full scalar low/high
words, completion, guards and exact lengths. Half policy markers 60043/61043/62043 and 1209 identify
contracts, not the current module version. Preserve all 27 raw buffers across the three modes,
including NaNs.

The checker independently encloses signed logarithms using directed integer recurrence and
same-width IEEE midpoint cells. Library admission reflects the existing empirical spacing/encoding
union for negative results: Float32 radii 1/1/2 for log/log2/log10, Float64 radius 1. Exact
special-input rules override approximation: either signed zero returns negative infinity, one
returns positive zero, negative nonzero inputs produce NaN, positive infinity returns positive
infinity, and NaNs require only NaN classification. NVVM Half narrows the Float32 admission set once.
CUDA Half checks the ideal RN-even result on this finite corpus. Bounded source/correction controls
distinguish its installed sequence without a complete PTX approximation model. A failed API check remains a
failure; do not fit tolerances to observed results.

CUDA Half log2 applies corrections to the evolving result. For log/log10, an encoded Float32
multiplication and Half narrowing precede corrections keyed by the original input. Preserve those
different evaluation orders; the installed-header checker owns their exact constants and stages.

CUDA double log10 separately models RN32 input conversion, Float32 log10 and widening because the
existing wrapper takes float. This is preservation evidence, not true-double accuracy. The selected
corpus excludes negative tiny doubles that narrow to negative zero, so its shared NaN classification
does not qualify that case. Require the independent checker, all three execution modes and exact
per-mode preservation; one is not a replacement for another.

Retain direct numeric60/61/62 rejection before mutation and legacy-text rejection before output
independently of module42 rejection. Immutable old-module metadata/rejection, source fallback,
fresh module43 loading and isolated static version checks complete the boundary proof. Initialize
the isolated static core cache before its scored identity capture; a reviewed unmatched-filter
invocation may initialize the session while executing zero tests, avoiding a duplicate version suite.
The final full native run and checkpoint may subsume matching focused selections as described in
WORKFLOW. Keep the seven earlier oracle suites and 21 fixtures unchanged.

## Report environment

Execution and comparison use the Python standard library. Shareable SVG/PNG charts use Matplotlib:

```bash
python3 -m venv build/nvvm-results-tools
build/nvvm-results-tools/bin/python -m pip install -r issue-nvvm-backend/requirements-results.txt
```

The report records the installed Matplotlib version. Use that environment only for report commands.

## Material compilation benchmark

```bash
python3 issue-nvvm-backend/nvvm-results.py material \
  --slangc build/RelWithDebInfo/bin/slangc --build-label RelWithDebInfo \
  --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
  --output "$NVVM_RESULTS/material"
build/nvvm-results-tools/bin/python issue-nvvm-backend/nvvm-results.py report \
  --measurements "$NVVM_RESULTS/material/measurements.json" --output "$NVVM_RESULTS/material-report"
```

The fixed material protocol runs six cells (two entries x NVRTC O3/NVVM O0/O3), two rounds in opposite
cell order, each with two warmups and nine measured fresh processes:132 compiles (108 measured,
24 warmup). Assembly is a separate eleven attempts per cell:66 assemblies (54 measured,12 warmup).
Every PTX/cubin hash must agree within its cell. No sample removal, automatic retries or cherry-picking.
Reports include median/IQR/range, per-round wall statistics and individual nested compiler phase
statistics. Keep raw diagnostics, including NVRTC PCH status when emitted. Warmups intentionally warm
filesystem/toolkit caches; a fresh process does not imply cold filesystem/PCH state. Record effective
math options and compiler/toolkit versions when comparing revisions. Unpaired historical sessions
are context, not a causal speedup claim. Run no build, GPU suite or profiler concurrently.

Automatic NVRTC PCH status is not necessarily visible in CLI logs: the driver appends its marker
to raw artifact diagnostics, while the ordinary CLI forwards parsed entries. An absent marker means
no directly observed PCH state; it does not prove the cache was disabled. Even a `not-created`
marker alone cannot distinguish reuse from a decision not to create. Label cache evidence as
observed, source-inferred or unavailable; keep normal cache behavior.

The material command does not explicitly select a floating-point mode. The current source maps this
to Slang Default, requesting neither `--fmad=false` nor `--use_fast_math`. Upstream adds
`--fmad=false` only for explicit Precise mode. Record actual commands and distinguish source-inferred
downstream options from an observed option trace. Changing math flags changes the experiment and
requires matching correctness evidence.

The existing bounded shared-session runner is a **separate experiment**, not interchangeable timing:

```bash
python3 issue-nvvm-backend/run-complex-corpus.py \
  --slangc build/RelWithDebInfo/bin/slangc --test-server build/RelWithDebInfo/bin/test-server \
  --build-label RelWithDebInfo --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so \
  --cuda-root "$CUDA_PATH" --warmup 2 --samples 9 --output "$NVVM_RESULTS/shared-material"
```

Retain `fresh_references`, `fresh_reference_seconds`, every batch and complete lifetime fields,
`runner_work_seconds`, and whole-command wall time. Request timers omit startup/reference/validation
cost; never present them as the end-to-end shared workload cost. The normal report command accepts
the maintained material/quality measurement schema, not this distinct shared-runner schema.

## Standalone corpus compilation

The current `compilation-performance` feature in [focused evidence](focused-evidence.json) pins the
reviewed standalone experiment, its manifest, runner and raw measurements. Its consumer is the
current performance comparison in STATUS and the feature matrix. Replace that feature on the next
reviewed refresh; keep scripts, exhaustive inventories, samples and presentations under ignored
`build/`, rather than adding permanent result snapshots. This experiment is separate from the
maintained material and quality commands above and below.

Freeze a reviewed mapping from each original test contract to a standalone `slangc` invocation.
Preserve entry/stage, row-major API default, language macros, module name and explicit semantic
options. Exclude binding-time specialization, conformance and shader-object contracts that the
standalone mapping does not represent; account for every excluded case. The current selection has
406 frozen and 98 discovery cases
(500 unique sources), with 76 exclusions. Only cases correct in all three original modes enter the
comparison; this is a selected workload comparison, not an estimate over all compiler inputs.

Use explicit `-g0` for all three modes. The renderer's implicit `-g2` requests NVRTC device debug
with optimization, while the direct route strips debug. Preserve original renderer options as
provenance, but do not include that asymmetric debug policy in a release comparison. The existing
GPU correctness evidence belongs to the original contracts. The timed release variants are
separately compile/assembly-qualified, not separately GPU-qualified.

Compile and assemble each selected cell before measurement, pin its PTX/cubin, and require every
timed PTX to equal the qualified artifact. Run serial fresh processes with warm filesystem/toolkit
caches. Within each fixed batch of at most 64 cases, use two rounds, reversing case and mode order
in round two. Each cell has one warmup and three measured samples per round. Keep all samples,
failures and timeouts; verify source/toolkit/compiler/provider/support-script identities before
and after each bounded batch. Assembly is outside the reported compilation interval.

Report per-case median, inclusive quartiles, range and round-specific medians. Aggregate the
NVRTC O3/NVVM median-time ratios using their geometric mean with equal weight per case. Keep
exclusions, slower cases and distribution plots visible. These ratios do not predict shared-session
application compile time, and quartiles do not establish statistical significance. Whole-census
elapsed time includes setup and validation; it supplies neither compile latency nor GPU performance.

## Original-input corpus dispatch timing

`extras/measure-nvvm-corpus-runtime.py` measures the original 452 frozen / 128 discovery contracts
through render-test. Use the qualified optimized harness, unchanged compiler/provider bytes and
the CUDA environment above. Run without competing builds, GPU suites or CPU-heavy reviews.

```bash
python3 issue-nvvm-backend/test-nvvm-corpus-runtime.py
python3 extras/test-cuda-dispatch-profile.py --output "$NVVM_RESULTS/dispatch-contracts"
python3 extras/measure-nvvm-corpus-runtime.py --output "$NVVM_RESULTS/corpus-dispatch"
```

Each invocation requires a new repository-local output path without whitespace or quotes, because
the test directive parser does not remove shell quoting. The runner preserves the original shader,
TEST_INPUT, dispatch size, specialization and output oracle in disposable mirrors. It explicitly
selects NVRTC O3/NVVM O0/NVVM O3 and release `-g0`. Final legacy compiler arguments are intentional:
render-test extracts uppercase `-Xslang` first, then appends legacy `-compile-arg`/`-xslang` options.
An original legacy `-O3` or `-g2` must not override the requested measurement mode.

The fixed protocol has two rounds with reversed case/mode order, three warmups and nine samples
per case/mode/round, plus one reference dispatch. A single test server executes each batch serially.
Native CUDA events bracket the RHI compute pass. **The interval includes the global-parameter
upload and host enqueue gaps; it is not kernel-only time.** Compilation, specialization, command
encoding, allocation, reset and output readback remain outside the events. Clocks are not locked;
brief workloads do not establish steady-state peak-clock performance.

The harness binds once and snapshots every registered buffer/counter and texture subresource.
Before every launch it restores the same allocations outside timing, preserving resolved addresses
and packed input data. It uses a fresh RHI command buffer with the retained root object. Every
repeat must match the reference's existing output serialization exactly; the ordinary test oracle
must separately accept that reference. Serialization can contain aggregate padding, so equality
can conservatively exclude cases. Focused contracts cover explicitly initialized per-invocation
static globals. The bare static aggregate in `nvvm-copyable-kernel-context` fails repetition in
both backends and remains excluded pending resolution of its initialization contract. Arbitrary
persistent external device globals are not a supported replay contract.

The runner attempts every selected cell, retains failed sidecars/logs, rejects incomplete or ambiguous
batch execution and never retries silently. `manifest.json` freezes the cases, options, schedule,
provenance and all mirrored inputs/oracles. `results.json` binds each cell to its original-oracle
result, sidecar, reference output and batch log. Reporting revalidates those bindings, hashes,
inventories and classifications. `per-case.csv` includes pooled and round medians; `samples.csv`
contains every accepted measured sample; `paired.csv` and `summary.json` retain complete pairs and
explicit exclusions. Regenerate them with `--report-only --output EXISTING_DIRECTORY` while the
recorded raw artifacts remain available and unchanged.

Keep short intervals visible. Ratios are withheld unless all three modes have both rounds and all
three pooled medians reach 0.1 ms. This is a conservative publication cutoff, not calibrated CUDA
resolution or proof of reliable speedup. Round ratios additionally require that round's medians to
reach the cutoff. Do not infer application throughput from these original correctness fixtures or
combine their dispatch intervals with the separately measured material kernels.

Changes to this shared harness or corpus runner require the full checkpoint and native/semantic,
toolkit and runner gates above, plus the focused replay contracts. Preserve exact old outcomes and
review the intentional harness identity transition before replacing accepted evidence. Raw attempts,
sample files and generated charts stay under ignored `build/`; current qualifications belong in the
`corpus-dispatch-performance` entry of focused evidence, replacing superseded observations in place.

## Original-input corpus code quality

Use `extras/capture-nvvm-corpus-code.py` to capture final specialized PTX through the same
render-test binding and entry-point path as the original-input dispatch experiment. This is a
fresh diagnostic execution with the original oracle and effective `-g0`/backend/optimization
options, not a new timing run. Existing `-dump-intermediates` and a unique per-cell prefix capture
the artifact returned by `getEntryPointCode`, which CUDA RHI loads unchanged. Exactly one PTX
module with one entry and target SM80 is required; dump ordinal numbers never choose a module.
Failures and ambiguous captures remain explicit. A previously failing cell that fails again is
not automatically proof of an identical diagnostic or failure mechanism.

```bash
python3 issue-nvvm-backend/test-nvvm-corpus-code-capture.py
python3 issue-nvvm-backend/test-nvvm-corpus-code-analysis.py
cp issue-nvvm-backend/focused-evidence.json "$NVVM_RESULTS/accepted-runtime-evidence.json"
python3 extras/capture-nvvm-corpus-code.py \
  --runtime-manifest "$NVVM_RUNTIME/manifest.json" --output "$NVVM_RESULTS/code-capture"
python3 extras/analyze-nvvm-corpus-code.py \
  --capture "$NVVM_RESULTS/code-capture/results.json" --output "$NVVM_RESULTS/code-analysis" \
  --runtime-summary "$NVVM_RUNTIME/summary.json" \
  --runtime-evidence "$NVVM_RESULTS/accepted-runtime-evidence.json" \
  --ptxas "$CUDA_PATH/bin/ptxas" --architecture 89 \
  --cuobjdump "$NVVM_CUOBJDUMP" --nvdisasm "$NVVM_NVDISASM"
```

`NVVM_RUNTIME` is the accepted dispatch collection directory. Set the two disassembler variables
to existing, versioned binaries; `cuobjdump` invokes `nvdisasm`, whose directory the analyzer adds
to its subprocess PATH. Record tool versions and hashes. The current experiment uses portable
NVIDIA CUDA 12.9 tools under ignored `build/`; it changes neither the installed toolkit nor the
compiler/harness. See NVIDIA's [binary utilities documentation](https://docs.nvidia.com/cuda/archive/12.9.1/cuda-binary-utilities/index.html).

The collector pins original selections, commands, test dependencies, runtime binaries and mirror
inputs; serial authoritative test logs bind each result to its case and mode. The analyzer consumes
only original-oracle-qualified captures and revalidates those bindings. It assembles with explicit
SM89 for the L4, obtains named-function hardware resources from `ptxas -v`, and disassembles the
cubin. All three modes use the assembler's default O3; NVVM O0 labels the earlier compilation
route, not an unoptimized assembler invocation. These are offline CUDA 12.9 results, not the
driver-JIT machine code recorded during timing.
Historical timed PTX was not retained, so matching source/options establishes the same compilation
route, not exact identity to those historical timed bytes. The older standalone 504-case CLI
compilation cohort remains a separate qualification.

Report entry and reachable-helper PTX instruction counts, typed virtual-register declarations,
branches, calls, shuffles and memory instruction families separately from hardware register
allocation, stack/spills, SASS instruction counts and executable bytes. PTX declarations are not
physical registers. Reachable totals count each resolved helper once, not once per dynamic call.
Unresolved calls remain visible. SASS counts include padding instructions and must exactly cover
all executable sections at 16 bytes per instruction for SM80/89. Missing data is never zero.

Raw PTX equality, normalized reachable instruction equality and complete named executable-section
byte equality are distinct observations. Normalization preserves literals, predicates, modifiers,
branch destinations and operand reuse while renaming generated symbols. It excludes declarations
and global data and is not semantic equivalence; scoped register redeclarations disable its exact
hash, and duplicate labels require explicit scope support. The heuristic similar-profile category
requires opcode-order similarity at least 0.9, SASS entry counts within 10%, hardware registers
within two, and equal stack/spills. This is triage, not a performance prediction.

`cases.csv` retains every case/mode outcome and metric; `pairs.csv` adds O3/O3 comparisons and
the separately accepted dispatch observations, with O0 retained in both tables. Report generation
recomputes metrics from pinned PTX, assembly logs, SASS and cubins. Timing joins require the exact
accepted manifest/summary hashes and case/corpus identities. Use `--report-only` with the same
arguments to regenerate tables from retained artifacts. Static size cannot account for loop trip
counts, mask activity, dependency chains or memory behavior; inspect representative source and
control flow before proposing a performance cause. Keep raw data and presentation packages ignored,
and replace the current focused code-quality evidence after independent review.

## Fixed simple-shader quality subset

```bash
python3 issue-nvvm-backend/nvvm-results.py quality \
  --correctness "$NVVM_BASELINE" \
  --slangc build/RelWithDebInfo/bin/slangc --build-label RelWithDebInfo \
  --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
  --output "$NVVM_RESULTS/quality"
build/nvvm-results-tools/bin/python issue-nvvm-backend/nvvm-results.py report \
  --measurements "$NVVM_RESULTS/quality/measurements.json" --output "$NVVM_RESULTS/quality-report"
```

The checked-in `quality-corpus.manifest.json` fixes12 representative existing shaders/36 modes and
source hashes. It includes integer/vector arithmetic, half/matrix values, helper transport, multiple
resources, row-major structured matrices, groupshared memory, switch/loop flow and inout mutation.
Its runtime IDs must have accepted three-mode correctness with identical source and compiler/provider
bytes. Explicit compiler options preserve row-major fixture selection. This is not the whole corpus
and does not assert runtime correctness of material shaders.

Compile each quality cell once, assemble with `ptxas -v`, record **named-function** register/stack/spill
resources and exact entry resources, retain optional `cuobjdump --dump-sass` and ELF section reports.
Absent metrics are null/unavailable, not zero. SASS instruction counts and executable `.text` sizes
cover the whole module. Cubin bytes include metadata; PTX bytes include formatting and symbol text.
Quality command latencies are single observations, not a repeated speed benchmark. None of these
metrics proves GPU speed, occupancy, numerical equivalence or a particular optimization's presence.

## Reporting and optional compiler-stage attribution

For a qualified instrumented material run, use the same reporter with an explicit option:

```bash
build/nvvm-results-tools/bin/python issue-nvvm-backend/nvvm-results.py report \
  --stage-attribution --measurements "$NVVM_RESULTS/material/measurements.json" \
  --output "$NVVM_RESULTS/attribution-report"
```

This adds `stage-attribution.json`, `.md`, `.svg` and `.png`. Keep the raw scope logs: the reporter
requires matching names, invocation counts and values, plus nested and outer interval containment.
It forms disjoint durations, remaining wall time and percentages per sample before computing medians
and inclusive IQRs; warmups are excluded and both rounds retained. Do not stack or sum marginal
medians. NVRTC exposes no separate verification call; its zero in that category is not proof of no
internal verification. The opaque vendor compile API is not pure optimization time.

Temporary attribution requires independently checked scope boundaries. Historical profiler patches
and packages are available through [HISTORY](HISTORY.md); they are not part of the current compiler.
Revalidate boundaries after source changes, preserve the accepted binary/module/cache layout, and
prove output preservation before interpreting timings. Restore the accepted layout at closeout.
Discarded instrumentation does not create a new full correctness baseline.

`report` produces readable/structured summaries and SVG/PNG figures under the new ignored output
root. Inspect generated charts and retain every sample there. Publish or export a presentation only
when requested; do not commit a new dated result package for every refresh. If a performance result
supports a current architectural decision, retain its compact reviewed metrics, provenance and limits
in the current evidence and matrix. Replace superseded summaries; Git owns earlier snapshots.

If a future source change invalidates the fixed quality manifest's hashes, review that fixture's
semantics and runtime obligations first. Update the manifest only as part of an accepted change,
then rerun quality against the matching correctness ledger. Do not simply replace hashes to make a
measurement pass. If the selected subset or options change, describe it as a changed experiment.

A results-only refresh of unchanged source can use the existing accepted-full ledger after verifying
its compiler/provider/input identities. A compiler, toolkit, shared runner or configuration change
requires the correctness gates prescribed by WORKFLOW before new claims. Preserve failed attempts
under the ignored results root. Update the current baseline/identity/focused evidence as applicable,
STATUS and feature matrix; keep completed working plans/report drafts uncommitted. Make the reviewed
local commit and stop unless further work was explicitly authorized.

## Scoped coherent pointer memory

Use the dedicated race-free fixture and the original groupshared discovery seed for focused memory
validation; preserve the original source and full checkpoint inventory:

```bash
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 1 -disable-retries \
  slang-unit-test-tool/nvvmIRBuilderCoherentMemoryPreservesScopesAndRejectsWithoutMutation.internal \
  slang-unit-test-tool/nvvmSlangCoherentMemoryUsesCheckedDescriptors.internal \
  slang-unit-test-tool/nvvmSlangCoherentMemoryRejectsBeforeProviderMutation.internal
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 1 -disable-retries \
  -api-only -api cuda tests/cuda/nvvm-coherent-pointer-memory.slang
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 1 -disable-retries \
  tests/cuda/nvvm-coherent-pointer-memory-unsupported.slang
python3 issue-nvvm-backend/run-compute-discovery.py --config RelWithDebInfo \
  --architecture 80 --jobs 1 --match coherent-load-store-groupshared \
  --modes nvvm-o0 nvvm-o3 --keep-mirrors --require-all-correct \
  --output "$NVVM_RESULTS/coherent-groupshared"
build/RelWithDebInfo/bin/slangc \
  tests/language-feature/pointer/coherent-load-store-groupshared.slang \
  -target spirv -entry computeMain -stage compute -capability vk_mem_model \
  -o "$NVVM_RESULTS/coherent-vulkan-control.spv"
```

Require three distinct units, two dedicated runtime cells, eight diagnostic cells and two original
source discovery cells. The SPIR-V compile checks that the alternative capability retains Vulkan
without imposing CUDA's SM floor. Inspect O0/O3 generated code for the exact GPU/global and CTA/shared
scopes, widths, memory effects and shared-address conversion. The direct provider unit compares both
serializers before/after rejected calls and rules out stores/RMW in load-only modules. Preserve
optional-interface absence and malformed-table negatives.

The original physical-storage-buffer and redundant-coherent-load fixtures contain racing accesses;
retain them as compile/optimizer evidence, not runtime oracles. Unsupported scopes in the latter may
remain rejected. Passing the groupshared seed does not resolve all coherent corpus failures. Scoped
relaxed accesses alone do not establish execution synchronization or acquire/release of other data.

## Explicit-layout pointer transport

Compile the durable Std430/Scalar/C fixtures at O0/O3 and retain actual CUDA Std430 rejection:

```bash
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 1 -disable-retries \
  slang-unit-test-tool/nvvmSlangLayoutPointersUseCheckedByteOffsets.internal \
  slang-unit-test-tool/nvvmSlangLayoutPointersRejectOtherRolesBeforeEmission.internal \
  tests/cuda/nvvm-layout-pointer-transport.slang \
  tests/cuda/nvvm-std430-pointer-transport.slang \
  tests/cuda/nvvm-layout-pointer-helpers.slang \
  tests/cuda/nvvm-parameter-group-layout-pointers.slang \
  tests/cuda/nvvm-pointer-reinterpret.slang \
  tests/cuda/nvvm-layout-pointer-fields.slang \
  tests/cuda/nvvm-layout-pointer-transport-unsupported.slang
```

The current allocated-address qualification uses the identical shader body in ignored
`build/nvvm-std430-pointer-transport/std430-pointer-transport.slang` and the existing ctypes CUDA
host pattern in `run-pointer-transport.py` in that directory. Compile strict NVVM PTX for both
optimization modes, then pass each output with `--ptx` and a fresh JSON destination with `--output`.
Bind three independently initialized 1024-byte allocations at interior offsets256/384/512 and a
guarded output allocation at offset256. For indices-1/0/1/2, require byte strides64/48/40, exact base
and shifted addresses, signed deltas, completion, every output guard and unchanged input allocations.
Check base and shifted alignment16/8/8. The shader never dereferences the records. Dumped LLVM must
sign-extend the index before scaling and use non-inbounds byte offsets; PTX must preserve the five
parameters d:u64, s:u64, c:u64, index:i32, output:u64.

The independent oracle follows the original pointer/data-layouts fixture: Std430 size64/alignment16,
Scalar field extent44 rounded to48, and C extent39 rounded to40. Earlier allocated Scalar/C evidence
under `build/nvvm-layout-pointer-transport/` retains CUDA's stride40 for both and three nonzero
Scalar mismatches. Actual CUDA Std430 use must reject at O0/O3 and minimum optimization before
storage lowering erases layout operands. CPP remains capability-rejected. Compile-only Vulkan,
LLVM shader IR and Scalar/C CUDA controls preserve their exact outputs. The initial host-style
LLVM target failure remains recorded separately from the corrected shader-IR control.

Internal-helper qualification reuses those exact allocations and output expectations through
`build/nvvm-layout-pointer-helpers/layout-pointer-helpers.slang` and its adapted host script.
The caller offsets the pointer once; a noinline forwarder observes it and passes it to a second
noinline helper that offsets it again. The moved word checks the caller offset, and the delta
checks the nested helper's independent offset. Require the second-step address to remain interior
and aligned. Both O0/O3 outputs must retain the six specialized helper bodies and their calls;
`[noinline]` is the recognized attribute. The first probe's unknown `[__noinline]` warnings and
corrected before capture are retained as a fixture incident.

Parameter-group qualification uses `tests/cuda/nvvm-parameter-group-layout-pointers.slang` and
`build/nvvm-parameter-group-layout-pointers/run-pointer-transport.py`. The three pointer values move
into a separately allocated 24-byte buffer, at offsets 0/8/16 and alignment 8. The eight-byte
`SLANG_globalParams` symbol holds that buffer's address; only index:i32 and output:u64 remain
kernel parameters. Verify reflection and emitted LLVM/PTX against this binding before launch.
Reuse the eight allocated helper cases, additionally requiring every parameter-buffer byte and the
module-global pointer to remain unchanged. Invariant pointer-field loads do not make the pointed-to fields immutable or permit whole-record
loads. Keep whole-group value, ordinary storage, pointer-array and unproven-root negatives, with
role checks before/after representation-cache population and preflight no-mutation assertions.

Run the direct static `nvvmLayoutPointerHelpersCheckEveryCallProducer` test in the existing isolated
static build, alongside the two `nvvmLocalRecordArray` controls. It places a same-typed unapproved
actual after a valid call to the same helper; no type-equality shortcut may admit the second call.
Keep static executable/configuration identity separate from the shared compiler and unit plugin.
Reinterpret qualification uses `tests/cuda/nvvm-pointer-reinterpret.slang` with that same host
and its `--source` argument. It compares explicit and reinterpret observations of the same allocated
addresses, including retained helper offsets. The direct static
`nvvmLayoutPointerReinterpretPreservesProducerChecks` test checks all three layouts and confirms
that normalization preserves rejection of an unproven root before provider mutation.
The original three-layout corpus fixture has its exact compile-only result in the focused record. Its unspecified bindings are
not an allocated-address oracle. Do not execute it or declare its corpus failure resolved from the
dedicated helper fixture.

Field-memory qualification uses `tests/cuda/nvvm-layout-pointer-fields.slang` and
`build/nvvm-layout-pointer-fields/run-pointer-transport.py --source` with that permanent source.
The binding and four signed indices remain the same. The host initializes four complete records
in each allocation, observes all ten scalar leaves per selected record before writes, and compares
all 3,072 pointee bytes after parity-selected stores. Even indices write the UInt64 field, selected
UInt32 fields, whole float3 and middle Bool; odd indices write the other UInt32 field, float3.y and
the outer Bool fields. Every padding byte, untouched neighbor and output guard must match.

The independent scalar leaf offsets (a.f0, a.f1, test1, b.x/y/z, test2, c/d/e) are
Std430 `0,8,16,32,36,40,48,52,56,60`, Scalar `0,8,12,16,20,24,28,32,36,40`, and
C `0,8,16,20,24,28,32,36,37,38`. Bool uses four/four/one bytes respectively. Inspect actual O0/O3
LLVM for the preserved kernel/group ABI, six noinline helpers, signed strides and scalar float3
payload accesses; a whole vector store must not touch a padding lane. Run the direct static
`nvvmLayoutPointerFieldsKeepLayoutAndProvenance` test and affected existing pointer/cache controls.
Its access negative proves exact qualifier preservation; readonly layout roots are not admitted.
The scoped negative selects actual SM80 and must fail at scoped-memory admission, not the SM floor.
GPU qualification covers the nested UInt64/UInt32/Float32/float3/Bool fixture; this is not an
exhaustive runtime qualification of every admitted numeric leaf combination.

## Physical surface correctness

`extras/validate-nvvm-surfaces.py` owns the independent physical-storage contract. The original six fixtures under
`tests/cuda/nvvm-surface-physical-*.slang` supply 83 cases: native Float32, signed/unsigned32 and
annotated Half, 1D/2D scalar/2/4 channels, whole/component writes, exhaustive scalar Half decode, converted NaNs,
dynamic-component writes, literal-rounding controls and zero-boundary operations. Four mixed-format
cases bind eight independent native Float32/r16f/rg16f/rgba16f source/result resources and check both
copy directions, with whole or component stores. Integer cases bind matching native 32-bit channels;
they do not qualify packed storage. The harness generates immutable input/expected
bytes per run, verifies reflected resource offsets and CUDA descriptors, assembles PTX, and copies
array bytes directly to the host. Shader roundtrips do not define its oracle.

```bash
python3 extras/validate-nvvm-surfaces.py --self-test
python3 extras/validate-nvvm-surfaces.py \
  --slangc build/RelWithDebInfo/bin/slangc \
  --provider build/RelWithDebInfo/bin --cuda-root "$CUDA_PATH" \
  --output "$NVVM_RESULTS/surfaces"
```

The full matrix returns nonzero while any requested case fails. Preserve every `(case, mode)` result;
NVRTC component compilation failures and its observed Half truncation differences remain recorded
limitations, never passing cells. Dynamic-component cases preserve the original physical oracle;
accept their intended NVVM failure-to-pass transitions only with complete physical execution.
Qualification is in-range, non-atomic whole-texel read-modify-write and grants no new out-of-range
lane or concurrent-write guarantee. Compare exact before/after identities when accepting transitions. `--cases` selects exact names from `--list`, and
`--modes` supports focused runs. Each run needs a fresh output directory. Raw byte arrays, PTX,
reflection, subprocess logs and diagnostics stay in the ignored output tree; current compact outcomes
belong in the accepted baseline; historical format and conversion failures remain in focused evidence.

Finite Half stores use ordinary RN-even conversion, including subnormals and overflow. Converted
NaNs require NaN class only; unchanged channels and guards require exact bytes. Run the CPU contracts
before GPU checks. The ordinary Half-cast regression in `tests/cuda/nvvm-half-narrow-conversion.slang`
checks conversions independently of surfaces; the math units exercise every finite Half midpoint.
After producer-tag edits, compare generated NVRTC CUDA source against the accepted
compiler with identical inputs/options. Check the declared semantic module-version boundary in
fresh sessions: explicit loads of incompatible historical user modules must fail with `E00130` before
AST/IR decoding, and modules rebuilt at the supported version must link against current built-ins
at O0 and O3.
Metadata inspection of rejected modules remains allowed. Preserve historical failure and wrong-branch
evidence when adopting an explicitly authorized compatibility break; rejection is not a successful
old-module load. Compatibility compilation is separate from fresh physical GPU evidence.

The standard `checkpoint` always runs this suite and uses `nvvm-surface-results.py` to validate the
complete cases × three modes inventory, source/oracle identities, phase return codes and diagnostics,
reflection, actual array descriptors, binding/upload/launch proof, cleanup and every host readback.
It writes `surface-comparison.json` and includes the validated `surfaces` block in `outcomes.json`.
The current per-cell obligations live in `accepted-baseline.json`; focused evidence retains format
semantics and earlier failure histories without duplicating that inventory. A raw harness exit 1
can preserve known negatives, but cannot pass checkpoint comparison without complete verified proof.
Converted NaN payloads are compared by class only; finite values, untouched channels and guards stay
exact. Known wrong-output signatures stay exact too.

Layered native32 rows are `native32-1d-array` and `uint32-2d-array-order`. The first groups
Float32/SInt32/UInt32 scalar, two- and four-channel resources; the second isolates array coordinate
order with aligned, in-bounds asymmetric coordinates. Array depth is independent of spatial height.
Four additional Half-array rows group native Half and formatted Float32/r16f/rg16f/rgba16f values
at both array ranks, widths 1/2/4, with whole or static-component copies. Independent source marker
writes prevent matching wrong load/store addresses from hiding corruption. Native copies and
untouched channels remain exact, including NaNs; converted NaNs use class-only comparison.
Two additional Half-volume rows reuse those conversion/marker contracts with ordinary XYZ
coordinates and explicit spatial depth. Volumes use height/depth with no layered flag; only
`array_layers` establishes an array role. The full host copy and report validator check these
roles independently. The shared component body prevents both CUDA volume entries from compiling;
keep this fixture limitation separate from CUDA whole-volume support generally.
Six additional integer-format rows group explicit signed/unsigned 8/16-bit storage, widths 1/2/4,
for non-array 1D/2D whole/static/dynamic-component accesses. Each binds twelve narrow arrays and
twelve independent native32 observation arrays. Representative signedness patterns establish load
extension; sixteen logical store edges per selected lane establish saturation before narrowing.
Full readback checks every untouched channel and guard. The oracle uses mathematical integer clamp,
not shader roundtrips or CUDA output. These rows do not qualify unannotated packed bindings.
Nine spatial integer rows extend the same formats and three store styles to 1DArray/2DArray/3D,
using the existing driver. Depth4 has two active planes and two guarded planes; height5 distinguishes
Y from Z. Independent plane-sensitive stores and host input markers prevent the tested coordinate
permutations from cancelling through matching reads/writes. Existing 97 inputs/oracles stay exact.
Fifteen native-narrow rows cover signed/unsigned8/16 logical values across the five geometries and
three store styles. Each groups twelve native resources and twelve independent 32-bit observation
outputs. Canonical inferred formats must agree with native scalar width/sign/channels in reflection.
Original raw inputs are independently sign/zero-extended; stores use in-range unsigned marker bits
and same-width signed bitcasts, with no surface narrowing/clamp. Require every selected marker
pattern per lane, untouched-channel/outer guards and bounded spatial permutation checks. Existing
106 source/spec/oracle contracts stay exact.
The current harness has 121 rows; the retained full baseline has 83. Preserve its 249 cells and use
the focused array/volume/integer evidence when reviewing the 114-cell expansion at the next full checkpoint.
The four new CUDA Half-array compile failures remain explicit comparison limitations.

A missing surface baseline, new/removed case, changed source/oracle, diagnostic or outcome requires
review. Bootstrap or expansion is a separate reviewed adoption of validated outcomes, never an
automatic allow-failures list. Preserve the old inventory and failure history during that adoption.

## Material runtime correctness

The standalone material validator qualifies the unchanged registered eval and sample entries with
synthetic 2×2 RGBA32F color/roughness textures and independent scalar references. It does not change
the compile-only scope of the complex-corpus runner. Native little-endian Linux, selected CUDA
headers, `c++` and a live GPU are required. Run CPU contracts first; select one entry per invocation
and use fresh directories for every attempt.

```bash
python3 issue-nvvm-backend/test-nvvm-material-runtime.py -v
export NVVM_MATERIAL_ENTRY=eval_buffer # Repeat the protocol with sample_buffer.
python3 extras/validate-nvvm-material-runtime.py \
  --slangc build/RelWithDebInfo/bin/slangc \
  --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
  --entry "$NVVM_MATERIAL_ENTRY" --prepare-only \
  --output "$NVVM_RESULTS/material-$NVVM_MATERIAL_ENTRY-prepare"
```

Before execution, review fresh PTX in all three prepared modes. Both parameterless entries use
168-byte align8 `SLANG_globalParams`, material offset80 with uint32 handles0/4, and count160.
The entry-specific layout is:

| Entry         | Input / output descriptor | Input layout                                                   | Output layout                                     |
| ------------- | ------------------------- | -------------------------------------------------------------- | ------------------------------------------------- |
| eval_buffer   | 96 / 112                  | stride40: UV0/4, wi8/12/16, wo20/24/28, seed32 (possibly dead) | stride16: value0/4/8, PDF12                       |
| sample_buffer | 128 / 144                 | stride24: UV0/4, wi8/12/16, seed20                             | stride32: wo0/4/8, PDF12, weight16/20/24, flags28 |

Confirm no live LUT reads for this fixed graph. Automatic checks cover symbols/strides, not arbitrary
register dataflow; supplying the reviewed reference below asserts that this explicit review has been
done. Fresh PTX and the oracle/input/tolerance hash must match preparation exactly. Verify the
compiler/provider/modules/cache/toolkit identities and retain provenance with the run.

```bash
python3 extras/validate-nvvm-material-runtime.py \
  --slangc build/RelWithDebInfo/bin/slangc \
  --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
  --entry "$NVVM_MATERIAL_ENTRY" \
  --abi-reference "$NVVM_RESULTS/material-$NVVM_MATERIAL_ENTRY-prepare/results.json" \
  --output "$NVVM_RESULTS/material-$NVVM_MATERIAL_ENTRY-execution"
```

Acceptance requires one real launch for each NVRTC O3/NVVM O0/O3 cell, 65 active records, 63 unchanged
guards, repeat/wrapped-UV checks, untruncated texture handles and successful cleanup. Eval checks
260 finite positive components against its scalar oracle. Sample checks 455 floats and 65 flags,
including exact32-byte zero rejection. Both use the frozen `1e-5 + 2e-4 * abs(reference)` budget.
Sample throughput is the selected layer's estimator, not full eval/PDF. Its two prequalified IOR
rounding candidates account for source cancellation; one must explain all records in a mode.
Never select models per component or widen tolerance after observing outputs. These are finite
candidates, not an exhaustive account of allowed compiler arithmetic. Preserve unexplained failures.

The default `--input-profile texel-centers` preserves the original input and oracle bytes. To
qualify off-center interpolation, add `--input-profile linear-filtering` to both preparation and
execution commands and use separate output directories. A preparation from another profile is
rejected even though the shader and ABI are identical. The four fixed locations exercise horizontal,
vertical and bilinear blends plus a footprint across both wrap seams. The oracle interpolates
uploaded encoded color before sRGB decoding and filters roughness before GGX arithmetic. Exact
dyadic weights avoid coordinate-weight quantization in these cases; arbitrary coordinates remain
unqualified. Wrapped-equivalence GPU records repeat the first location only.

The oracle/inputs/tolerance are written before launch. `prepared` is compile/assembly only;
`passed` additionally includes execution and comparison. This is correctness evidence, not a timing
command. Original assets, live LUT reads, arbitrary graphs/inputs, sampling-distribution accuracy
and application performance remain unqualified. Update the entry's existing focused feature after
independent review, retaining failures. Keep raw outputs, snapshots and audit logs under ignored build paths.

## Material device-event measurement

Use fresh successful **texel-center profile** eval and sample runtime reports from the preceding
protocol. The fixed measurement contract rejects linear-filtering profile reports. Their artifact
hashes must still match the current driver, validator, manifest, compiler and shader bytes. The
measurement runner compiles nothing; it reuses the qualified cubins and shared helper. Run with the
same CUDA environment and no competing builds or GPU work. Every output directory must be new.

```bash
python3 issue-nvvm-backend/test-nvvm-material-measurement.py
python3 extras/measure-nvvm-material-runtime.py prepare \
  --helper "$NVVM_RESULTS/material-eval_buffer-execution/material-driver" \
  --eval-reference "$NVVM_RESULTS/material-eval_buffer-execution/results.json" \
  --sample-reference "$NVVM_RESULTS/material-sample_buffer-execution/results.json" \
  --output "$NVVM_RESULTS/material-device-protocol" --timeout 120
```

Review the pinned protocol, helper and already reviewed PTX before launching. The fixed protocol
uses unchanged 65-record tiles at 65,537 and 1,048,577 active records, 64 threads per block and 63
guards. Six small helper-regression cells must pass before twelve enlarged qualification cells.
The independent oracle checks the first tile; every remaining active byte must repeat its verified
record. Sample candidate selection remains global, and all rejection records must be exactly zero.

```bash
python3 extras/measure-nvvm-material-runtime.py qualify \
  --protocol "$NVVM_RESULTS/material-device-protocol/results.json" \
  --output "$NVVM_RESULTS/material-device-qualified" --timeout 120
```

Independently review all eighteen qualification outcomes, raw outputs and their bindings before
measurement. Each full reference is bound to entry/count/backend/cubin/input/oracle/layout. The
measurement stage rechecks those bindings and independently requalifies the reference bytes.

```bash
python3 extras/measure-nvvm-material-runtime.py measure \
  --protocol "$NVVM_RESULTS/material-device-protocol/results.json" \
  --qualification "$NVVM_RESULTS/material-device-qualified/results.json" \
  --output "$NVVM_RESULTS/material-device-measured" --timeout 120
```

Each of 24 cells (two entries × two counts × three modes × two rounds) retains three warmups and
nine measured launches. Round two reverses the complete cell order. Reset all active/guard bytes and
synchronize before each launch; after the stop event, download and compare every byte against the
qualified reference. Retain mismatch buffers, logs, every timing and every failure. Identical
per-launch buffers need not be duplicated when the complete comparison evidence is retained.

Report `device_ms` separately from `host_submit_ns`, `host_until_stop_ns` and fresh-process wall time.
CUDA12.9 `cuEventElapsedTime_v2` measures one launch's event interval, which can include submission
gaps. It excludes compilation, allocation, reset, readback and comparison. Require complete passing
launch/output/cleanup evidence; observed competing processes suppress accepted summaries. Before
and after process snapshots cannot exclude transient contention. Record changing clocks and device
state without controlling them.

Keep median, inclusive quartiles, range and round-specific ratios. The fixed 0.1 ms interpretation
threshold suppresses records/second when any measured interval is shorter. This is a conservative
protocol rule, not a CUDA accuracy guarantee. Keep noisy samples and order effects. Sampling includes
1,008 / 16,131 rejection records at the two counts (64,529 / 1,032,446 non-rejected records).
Tiny 2×2 textures, periodic coherent inputs and correctness transfers between launches are central
limitations; this does not measure continuous rendering or application frame performance. Static
stack/register differences may motivate follow-up investigation but do not establish causality.

### Generated-code controls

The current eval-only local-store diagnostic lives in the existing device-event feature record.
Its transformed PTX removes exactly 65 whole `st.local` instructions from the pinned NVRTC artifact.
An independent complete local-address audit proves that no pointer escapes and all memory accesses
use explicit state spaces. Protect the entire read intervals `[320,344)`, `[368,384)`, `[408,448)`;
keep any vector store touching any protected byte. Retain all six loads, 29 overlapping stores,
the 592-byte local declaration, and every other PTX byte. Reassembly uses the identical pinned
`ptxas -v -arch=sm_80` command. This is an exact-artifact diagnostic, not a reusable compiler pass.

The control compares unchanged NVRTC, transformed NVRTC and unchanged NVVM O3 for eval only.
Three 65-record prerequisites and six enlarged correctness cells precede twelve timing cells
(two counts × three arms × two reversed rounds), each with three warmups and nine samples. Reuse the
maintained driver/oracle and every output/reference/provenance obligation above. Pin proof support
hashes before optional artifacts are merged, and verify that reinserting the removed lines exactly
reconstructs the original PTX. Preserve all attempts and keep source/proof snapshots under `build/`.

The accepted control removes most of the large-count eval timing gap while retaining exact outputs.
Removing stores also permits downstream register, scheduling and aggregate optimizations, so it
cannot prove a unique memory-traffic bottleneck. No sample transform, SASS or profiler evidence was
collected. A production proposal needs a reduced producer/optimizer reproducer and general semantic
proof; do not ship a workload-specific PTX deletion based on this experiment.

For source reduction, first replay the exact generated CUDA through the installed Slang adapter:

```bash
build/RelWithDebInfo/bin/slangc GENERATED.cu -pass-through nvrtc \
  -entry eval_buffer -stage compute -target ptx -capability cuda_sm_8_0 -O3 -o REPLAY.ptx
```

Require byte-identical baseline PTX before interpreting variants. Pin the compiler library, NVRTC,
adapter source, prelude, assembly tool and every candidate; retain the leading prelude include and
identical options. The current bounded inventory is 16 synthetic variants (initialization, receiver
snapshot, branch spelling and indexing) plus three exact full-source inline-hint controls, following
one original replay. All compile and assemble. API options are inferred from adapter source, not
intercepted. Compile serially with 120-second command and 30-minute total bounds; retain failures and
blocked cells. Sources, runner and logs remain under ignored `build/nvvm-material-reduction/`.

A static positive requires complete local-address/noescape/read-range proof, including bounded
indices and loop trips. The reduced bounded-index PTX copies `[288,428)` into `[444,584)` and reads
only `[444,460)`, `[464,472)`, `[476,480)`, `[488,492)` from that copy. Twenty-seven stores outside
all read ranges are never read; one initializes 50 words in a loop, giving 108 static store bytes
and 304 executed store bytes per active thread. Preserve live local data. Constant-index variants
have only explicit global/parameter memory accesses and no local declarations or calls. Stack and
register reports remain separate observations; no reduced-source GPU or speed claim follows.

## Documentation and evidence maintenance

[accepted-baseline.json](accepted-baseline.json) owns full comparison outcomes and native identities.
[accepted-identity.json](accepted-identity.json) preserves the current verified runtime/layout/config
snapshot and dependency pins. [focused-evidence.json](focused-evidence.json) owns selected later
qualifications and open failures outside the full corpus; it does not change full-checkpoint cadence.
Their historical provenance/loop fields never override STATUS/WORKFLOW authority.

Replace the accepted baseline only after reviewed full validation, identifying its predecessor by
Git revision, path and hash. Older repository paths in retained failure histories are archival
citations resolvable through HISTORY; `compare` reads embedded rows without opening those references.
Local `build/` evidence may be absent in a fresh checkout. Current runner inputs must exist
independently of both raw evidence and Git archives.

Retain `census.slice-195.tsv`, `census.slice-146.tsv` and `census.slice-146-clusters.json`: their old
names identify live frozen selection/overlap/summary inputs, not disposable reports. Preserve their
bytes unless a separately reviewed inventory change explicitly authorizes otherwise. Discovery,
material and quality manifests remain current input authorities.

For summary utilities, set output paths under `build/` explicitly (in particular the discovery
summarizer's `--table` and `--clusters` options); legacy defaults can generate numbered files in the
issue directory. Ignore guards prevent accidental snapshot commits. Keep source/tool behavior changes
separate from documentation cleanup. A documentation-only migration needs exact data/input checks,
link/reference checks and relevant CPU contracts; it does not require a fresh GPU checkpoint.

### Relinking and repeated static context

Use the existing focused selectors `nvvmSlangLinkedRouteOverridesEmitIsolatedImplementations`,
`cudaEmissionMethodLinkOptionsAffectRoutingAndHash` and `SlangcReadFromStdin` plus the three
`tests/cuda/nvvm-copyable-kernel-context.slang` cells. The route test compiles the original before
relinking, switches an option-bearing variant back, and checks original hash/layout/code stability
and inherited options. The static fixture explicitly initializes its record with `{}`.
For the retained third-dispatch failure, reuse the corpus driver's allocation reset and output
comparison with one reference, one warmup and one sample launch in each mode. The ignored
`build/nvvm-link-options/repeat-static-context.py --output <fresh-directory>` adapter also checks
the exact32 uint32 values6400..6431. Event fields are protocol evidence only, not accepted timing.

### Restricted CubeArray and mip-query investigation

The raw probes under `build/nvvm-texture-dimension-investigation/` use CUDA12.9, target SM80 and
the recorded L4/driver identity. Compile the two cube probes with nvcc (`-lcuda` for the direct
driver variant); both retain null/full controls, independent face markers, descriptor readback and
guards. The restricted 6..23 case deliberately retains depth 18 versus expected 3 as a mismatch,
even though its six content samples pass. No count correction is qualified.

Assemble the three isolated query PTX files with ptxas and build the host with g++/cudart/cuda.
The host tests both PTX and cubin against full 8×4 mip geometry and a checked mips 1..2 view. Run
once normally and once with `CUDA_MODULE_LOADING=EAGER`; record module-load, kernel-lookup and
execution outcomes separately. Width has 12 guarded successes per loading mode; the other two
opcodes have 8 failed materialization attempts per mode (driver error 500), not skipped/passing executions.
`readelf -sW` confirms kernel symbols and the failing cubins' extra weak undefined descriptor-size
symbol. No private symbol value is supplied. These are standalone CUDA/PTX mechanism results,
not fresh Slang capability or full-corpus acceptance. All earlier failures remain retained.
