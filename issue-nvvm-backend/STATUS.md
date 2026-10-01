# NVVM current status

The CUDA-text route migration is **complete with focused local acceptance**. NVVM no longer
infers operations from CUDA assembly-body strings or active semantic tags. The final field-offset
recognizer is replaced by a typed query that preserves the exact field key before optimization.
Explicit LLVM/libdevice names and genuine primitive PTX remain intentional backend interfaces.
Module43, ABI46 and container2 are unchanged. The signed16 O3 failure is corrected by provider-side
normalization at exact-width integer consumers. The consolidated integration checkpoint is accepted;
operation dispatch, shared type-role admission, structured-buffer planning, fake-provider maintenance
and architecture refresh are accepted. Native Half ceil/floor/trunc and single-rounded FMA are
accepted. The requested full validation checkpoint is accepted; standard feature work resumes below.

The accelerated workflow authorized on 2026-09-30 remains in effect: eight build jobs, larger
related batches, focused compile/PTX/runtime checks and no routine module-version bumps or full
campaigns. The maintainer superseded the earlier stop-after-log request. The maintainer has resolved the
offset scope: preserve existing NVVM restrictions and leave CUDA/CPP behavior unchanged. The final
offset migration is accepted. The maintainer has now authorized the ordered correctness,
integration, cleanup and Float16 work below; continue through bounded reviewed local commits,
stopping only for a decision that actually needs human input. After all queued cleanup and Float16
work, the maintainer additionally requests a full NVVM validation checkpoint, fixes for discovered
issues, and resumption of feature development under the standard workflow. No push, Slack or system changes.
Preserve the user's untracked `tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw full evidence is under ignored
`build/nvvm-integration/full-after-cleanup-1/`; Half evidence remains under
`build/nvvm-half-native/` and earlier cleanup evidence retains its recorded paths.
Plans and reports remain uncommitted.

## Current full acceptance

The requested post-cleanup/Half checkpoint is accepted after independent review.
All exact corpus and surface outcomes match the earlier full baseline:

| Validation             | Result                                                                                                         |
| ---------------------- | -------------------------------------------------------------------------------------------------------------- |
| Main corpus            | 1,704 correct / 36 known unresolved across 1,740 cells; 21 resolved histories preserved                        |
| Physical surfaces      | 214 pass / 24 known compile failures / 11 known NVRTC mismatches across 249 cells                              |
| Native units           | 1,173 pass / 12 inherited skips; four stale assertions repaired and retried                                    |
| Semantics / capability | 1,171 pass / 78 inherited skips; 106 capability tests pass                                                     |
| Toolkit / material     | 18 toolkit, 6 material compile/assembly and 12 material runtime cells pass                                     |
| Runtime / numerical    | Main runtime 4; supplemental 18 fresh plus mapped/reused coverage; numerical 99 fresh + 3 qualified Half cells |
| Static / contracts     | 5 direct static units; 119 runner contracts + 83 surface contracts pass, one inherited contract skip           |

No unexpected compiler/runtime regression remains. The full native run initially had four stale
call/diagnostic assertions; exactly four focused retries pass after test-only corrections. One
supplemental surface diagnostic similarly expected a deferred field error instead of the earlier
`imageLoad` rejection; only its expected diagnostic changed, and its retry passes. Initial failures,
exact inventory changes, test-only identity differences and unchanged no-mutation checks are retained.
The checkpoint does not claim every backend's full repository suite or publication/CI readiness.

Native Half ceil/floor/trunc emit directed Half conversions. FMA uses RN16(exact(a*b+c)), correcting
3e02 double rounding to 3e01 for the motivating inputs. LibNVVM12.9's trunc limitation requires
checked Half/i16 transport around `cvt.rzi.f16.f16`; whole-module verification remains enabled.
These instructions require SM53 or newer; this batch is qualified at SM80 and SM50/52 is unqualified.
Round ties and approximate exp2/tanh remain separate documented policies. CUDA is comparison
evidence, not an automatic oracle; retain the documented Vulkan/D3D12 contract distinctions.

## Full checkpoint identity

The full run used repository revision `d35654675` after test-only repairs. Production compiler
source is revision `593a686f79ea51e3248373f9aaf47e1593140e32` plus implementation patch
`a8d5e12e3d2b5b795d17dfbe0fcd1eb9b645fb1cc1a7cc324ef42971f0998ce8`;
see accepted identity for authoritative full revision/hash values. The configured compiler string
remains `2026.18.3-350-gdc0a9acc3`; later commits do not relabel binaries.
Compiler SHA-256: `1a9a6c04b7414f47fc74cdb0e0f13293085a2637f1e3e6ea8899af1bdd4547e9`.
Provider SHA-256: `9ace32f8b44d19e9e09256f7f58555a24b262a429f238ce9b51576870bab83c6`.

The [accepted baseline](accepted-baseline.json), [accepted identity](accepted-identity.json) and
[focused evidence](focused-evidence.json) retain exact current outcomes, all 42 earlier feature
objects, input/binary/configuration hashes and precise reuse. Earlier performance and other
historical claims retain their original identities. Raw current full evidence is under
`build/nvvm-integration/full-after-cleanup-1/`.

Environment: native Ubuntu 24.04, eight logical AMD EPYC CPUs, about 30 GiB RAM, L4 SM89,
UUID `GPU-7e9accb6-0e0f-7bb1-cafe-c1d02947b736`, driver 595.71.05, CUDA 12.9.2 / NVRTC 12.9.86,
LLVM 14, target SM80. CPU-only checks overlapped the main GPU checkpoint; no performance claim.

## Current feature and next action

The migration, ordered cleanup, native Half batch and requested full checkpoint are complete.
Resume bounded reviewed feature work under WORKFLOW, retaining eight-job incremental builds and
economical focused validation. No automatic full campaign follows each feature.

Fourteen bounded feature batches now pass focused validation after the full checkpoint:

- UInt2 low/high word transport for selected read-only texture handles resolves the two NVVM
  `gh-6657-nonbindless-uniform` discovery cells. Three units, three runtime cells, four existing
  buffer negatives and record layout checks pass. Unsupported resource roles remain excluded.
- In-range dynamic surface-component stores resolve four NVVM physical cells. Eight physical
  runtime cells, two distinct units and four existing negatives pass. The merge converts only the
  replacement and preserves untouched physical lanes. A fake Float32/Boolean classification bug
  and an overbroad call-count assertion were fixed; original attempts remain recorded.
- Internal mutable and readonly references to existing local record arrays support nested forwarding
  and source returns through canonical OutParam lowering. The mutable batch passes three shared,
  two static and nine runtime cells; readonly passes four shared, two static and nine runtime cells.
  Readonly access preserves caller mutation visibility and cannot grant writes. Native array/pointer
  results, external roles and unproven pointer roots remain excluded.
- Non-array, non-mip texture dimensions now accept Float32 outputs through ordinary numeric casts.
  One typed-operation unit and three runtime modes pass. Four retained mip/MS negatives pass after
  correcting stale expectations to the existing earlier capability/type diagnostics; failures retained.

The full baseline retains its original 36 corpus gaps and four dynamic-surface failures; focused
records supersede the affected cells without relabeling the full run. Current identities and
precise reuse are in `features.nvvm-texture-descriptor-words`,
`features.nvvm-dynamic-surface-components`, `features.nvvm-local-record-array-references`,
`features.nvvm-local-record-array-borrows`, `features.nvvm-texture-float-dimensions`,
`features.nvvm-texture-array-layer-counts`, `features.nvvm-texture-1d-array-layer-counts` and
`features.nvvm-native-array-surfaces`, `features.nvvm-half-array-surfaces` and
`features.nvvm-half-volume-surfaces`, `features.nvvm-coherent-pointer-memory` and
`features.nvvm-layout-pointer-transport`, `features.nvvm-integer-surface-formats` and
`features.nvvm-integer-array-volume-surfaces`. Fourteen
implementations have passed focused validation since the full run.

Actual non-mip Texture2DArray layer counts now pass focused validation for int/uint/float outputs.
A separate scalar query preserves spatial rank and maps to the existing provider depth query with
the real 2DArray descriptor. Three units, five runtime cells and four unchanged mip/MS negatives pass.
The Slang O3 raw-handle probe also returns 5/3 for explicit full/restricted views of one 11×7×5
allocation, with exact guards and independently checked view descriptors. CUDA C++ still returns
zero; this discrepancy is documented rather than used as the oracle. The null-view getter anomaly
and singleton host-binding limitation remain separate and unresolved.

Non-mip Texture1DArray dimensions now also pass all three output scalar families, using the
existing scalar count query mapped to provider height. Three units, five runtime cells and four
unchanged negatives pass; Slang O3 explicit full/restricted views return width/count 11/5 and 11/3.
The eight-job incremental build took 35 seconds. Stable916 and module/ABI/container versions remain
unchanged; fourteen feature implementations have passed focused validation since the full checkpoint.

Native32 1D-array surfaces now pass independent physical checks for Float32/SInt32/UInt32
scalar, two- and four-channel loads/stores. The same provider change corrects existing 2D-array
argument order: the layer precedes spatial coordinates only in the LLVM call. Before the fix,
logical (1,4,2) accessed physical (1,2,4); exact O0/O3 failures are retained. Twelve focused physical
cells, including non-array native/Half controls and CUDA comparisons, pass. Four distinct units,
four retained diagnostics, 85 harness cases and 22 report contracts pass. The fake needed a valid
constant-record admission fix, and new coordinate assertions needed correction; all attempts remain
recorded. Production binaries stayed unchanged through those test-only retries. The eight-job build
took 28 seconds. Original 83 surface case/oracle identities and the full 249-cell baseline are
unchanged; two new grouped rows are recorded separately for the next reviewed full expansion.

CubeArray binding remains unqualified. The first explicit-view probe returned depth 30/18
instead of intended cube counts 5/3. A separate content probe then tested endpoint indices in cubes:
null-full and explicit 0..4 passed both count and all source-cube samples, while restricted 1..3
reported count 3 but sampled faces shifted by one face, not one cube. Exact observations are retained
in the 1D feature's research history. Matching counts/getter echoes do not establish intended
selection; neither division by six nor another endpoint guess is justified. No CubeArray admission
change follows these failures. Continue with another qualified feature while this binding contract
remains open. Mip-count linkage histories and the full dimensions corpus failure remain unresolved.
No new full campaign.

Native and formatted Half surface storage now passes for 1D/2D arrays, widths 1/2/4, with
whole/static-component updates. Eight new NVVM O0/O3 physical cells pass; the four corresponding
CUDA compile failures are unchanged. Two non-array controls add five passes and one retained CUDA
compile failure. Four units, four diagnostics and 89 harness contracts pass. All original 85
case/oracle identities are preserved. The eight-job build took 35 seconds; no conversion policy,
module version or ABI changed. The full surface baseline retains its original 249 cells.

3D Half surface storage now passes the same native/formatted representations and widths with
whole/static-component updates. Four new NVVM physical cells pass; six existing array/non-array
control outcomes remain exact. Two CUDA volume fixture failures retain the shared helper's invalid
component subscripts, without claiming all CUDA whole-volume operations fail. Four units, four
unchanged diagnostics, 91 harness cases and 22 report contracts pass. Volume depth and array role
remain distinct, with independent host readback and exact spatial guards. The eight-job build took
34 seconds. The full baseline remains unchanged; eight new rows require reviewed adoption of
24 cells at the next full checkpoint.

Scoped coherent pointer memory now passes for naturally aligned Int/UInt32/64 with exactly
Device/global and Workgroup/shared accesses. Three distinct units, four race-free O0/O3 GPU cells,
eight negative cells, two LLVM/PTX inspections/assemblies and a Vulkan compile control pass. The
original groupshared source is unchanged and its two NVVM failures are resolved in focused evidence.
The physical-pointer fixture compiles but is not executed; redundant-load still rejects unsupported
scope. Both original programs contain races, so their runtime failures are not declared resolved.

Focused testing found and fixed three production gaps: pointer offsets now inherit the canonical
base layout at the buffer-layout producer, explicit target bodies prevent CUDA from silently emitting
ordinary loads, and preflight checks actual SM70 capability rather than trusting source-profile
upgrade warnings. The original failed outputs are retained. Fake pointer fields and per-global type
recording also needed repairs; only the test plugin was rebuilt for those final retries. The successful
production compiler checks were reused with exact identity. Optional interface6 leaves existing ABI46
tables unchanged. Other scope/space pairs, new pointer roles and volatile semantics remain excluded.
Research and all attempts are under `build/nvvm-coherent-pointer-memory/`; no new full campaign.

Explicit Scalar/C layout Device record pointers now support entry transport, signed32 offsets and
one-way UInt64 address observation. Eight allocated-address O0/O3 cases pass, with independent
48/40-byte strides, guards and unchanged inputs. Four distinct units, four compile/negative cells
and two existing coherent runtime controls pass. One negative used the nonexistent public Generic
address-space name; its corrected GroupShared case passes with unchanged no-mutation assertions.
The first attempt remains recorded. Compiler build took33 seconds with eight jobs; only the test
plugin was rebuilt for the fixture correction.

Shared layout selection owns the C physical record and Scalar stride; preflight retains the exact
stride and entry root, and emission uses non-inbounds byte offsets. Record dereferences, helpers,
pointer storage/reconstruction and Std430 remain excluded. CUDA C++ uses stride40 for both layouts,
failing the three nonzero Scalar cases; retain that discrepancy rather than changing the oracle.
CUDA source and Vulkan SPIR-V controls are byte-identical before/after. The original three-layout
corpus test remains unresolved. Evidence is under `build/nvvm-layout-pointer-transport/`.

Explicit signed/unsigned 8/16-bit surface formats now pass for non-array 1D/2D, widths 1/2/4,
with whole, static-component and in-range dynamic-component stores. Loads extend to matching
32-bit shader values; stores saturate before narrowing. Twelve new NVVM O0/O3 physical cells,
four units and four retained diagnostics pass. Two existing integer32/Half controls retain their
six exact outcomes. All six new CUDA comparison cells still fail compilation because integer
surface-conversion helpers are missing; no CUDA runtime equivalence is claimed.

The harness now has 97 rows, preserving all 91 earlier input/oracle identities. Its CPU contracts
and 22 report checks pass. The build initially caught a misspelled enum in a test assertion;
correcting that token completed the eight-job build in21 seconds. Production needed no build fix.
D3D's narrower-integer clamp contract owns this policy; Vulkan load interpretation agrees, while
out-of-range storage-image encoding equivalence remains unqualified. Raw evidence is under
`build/nvvm-integer-surface-formats/`. Unannotated packed bindings and normalized formats retain their separate boundaries.

The same explicit integer formats now also pass for 1DArray/2DArray/non-array3D, completing the
existing native32/Half geometry family without changing conversion code. Eighteen new O0/O3
physical cells and four retained control cells pass; nine new CUDA compile failures match before,
and both control CUDA failures remain exact. Four units, four diagnostics, 106 harness contracts
and 22 report contracts pass. All 97 prior source/oracle identities are preserved. The eight-job
build took37 seconds with no retry. Evidence is under
`build/nvvm-integer-array-volume-surfaces/`. Fourteen feature batches are now accepted since the
full checkpoint; its 249 surface cells remain unchanged, with 69 new cells awaiting reviewed
adoption at the next deliberately selected full checkpoint.

Next, select the next bounded feature from the remaining physical surface/type contracts. Std430
source admission, CubeArray restricted views and mip queries retain their separate research gaps.
Continue focused reviewed batches; do not rerun the full checkpoint merely for this feature.

The generic `requirePrelude` source-text boundary, graphics entry tests and hardware capability
failures are separate from missing NVVM primitives.

Approximate Half exp2/tanh remain a separate accuracy/capability choice; other transcendental
policies stay unchanged until supported by evidence. Dynamic surface indices remain in-range
qualification with non-atomic whole-texel RMW and no new out-of-range guarantee.

## Retained boundaries

Module43 requires older user modules and separately supplied built-ins to be recompiled for every
backend. Metadata inspection and source fallback remain available. The active NVVM semantic-tag
extension is removed; inert serialized slots and ordinary explicit intrinsic arguments remain.

Preserve all 36 main gaps and focused NVRTC narrow-bit/nested-array failures and timeouts.
Packed/normalized surfaces, general aliases, broader resource provenance and three-channel
transfers remain outside current physical legalization. Checked address/memory plans
remain authoritative; structured-buffer load/store conversions are planned, while other resource
family planning and broader aggregate admission remain feature work.
Barrier convergence, external Half ABI, numeric sweep, material-runtime and performance conclusions
retain prior qualifications. Snapshot caching remains non-atomic with external libdevice replacement.
Repeated bare-static-state dispatch in nvvm-copyable-kernel-context remains unresolved. Relinking a
compiled requirement-free component may retain cached target output after option changes.
CUDA `dim3 == uint3` source emission remains unsupported.
