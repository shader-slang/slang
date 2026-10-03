# NVVM current status

**Scalar-condition vector select repaired (2026-10-03):** the shared NVVM admission rule now
accepts scalar Boolean predicates for existing numeric/Boolean vectors, reusing the current lane
compatibility helper and LLVM emission. All **nine standalone compile/assembly cells pass**
(four rejections resolved, five controls preserved), focused checks **6/6**, NVVM units **570/570**
and smoke **16/16** pass. GPU coverage spans both predicate values, widths2-4, Boolean, signed/unsigned
integer8/16/32/64 and float16/32/64 at NVVM O0/O3 and NVRTC O3. No ABI/module bump or new lowering path.

**Falcor2 follow-up:** NVRTC still renders successfully (**1 pass, 12.81 s**). NVVM gets past the
original select rejection but reports **1 fixture error, 8.17 s** at
`WaveActiveSum(uint64_t3)` in `emissive_geometry_kernels.slang:271` (`E41400`: unsupported NVVM
partition operation scalar type). Path tracing is still not reached. All three routing controls
per backend pass with refreshed compiler/provider identities. The initial select failure and
comparison are retained in [application status](falcor2-status.json); the standalone history lives
in [the corpus manifest](application-corpus.manifest.json). This bounded select fix is complete;
64-bit wave reduction, unrelated Torch failures and the general feature loop remain stopped.

**Two easy Torch families repaired and stopped (2026-10-03):** CUDAKernel direct calls now use
ordinary helper clones, and explicit parameter-group pointer loads retain their global-memory
provenance for existing helper conversion. All **five mapped failures now pass**, with **49 prior
neighboring passes preserved** and **five NVVM route controls passing**. Focused compiler/runtime
checks pass **16/16**, units **570/570**, and smoke **16/16**. Local corpus replay is **15 pass /
two polynomial-layout rejections / one softplus O3 timeout**. No provider change, ABI bump or working
tier admission. The six polynomial application failures and softplus timeout remain unresolved;
this is focused acceptance, not a new full Torch/SlangPy checkpoint. General development stays stopped.

**Application reproducer corpus added (2026-10-03):** four portable Torch-derived shaders now
isolate all four newly exposed NVVM diagnostic/timeout families, with two passing controls.
The existing runner covers 18 compile/assembly cells. The bounded repairs above supersede the
original corpus-only stop for two families; the general loop remains stopped. See [current Torch findings](#torch-findings-and-local-application-reproducers), the
[manifest](application-corpus.manifest.json), [workflow](WORKFLOW.md#application-derived-reproducers)
and [replay command](RESULTS.md#application-reproducer-corpus).

**Compact-vector consolidation complete and stopped (2026-10-03):** explicit and default
uniform groups now share the existing CUDA storage lowering. Both duplicate compact-vector load
conversions are removed. Host layout, Half role separation, snapshot semantics and material
performance are preserved. Full working **1,735/1,735** retains all 1,726 prior passes; units
**570/570** and smoke **16/16** pass. Material PTX is byte-identical to the accepted fast code;
no fresh timing is claimed. This bounded request is complete; no general loop or wave work resumes.

**Material regression repaired; bounded work complete (2026-10-03):** shared storage lowering
now unpacks ordinary NVVM aggregate snapshots at their original read, using its existing helpers.
The small producer-side fix adds no provider machinery or ABI change and fits later storage/ABI
cleanup. At 1,048,577 records, matched before/after material eval is **2.741 → 0.313 ms** and sample
**2.381 → 0.391 ms** (8.75×/6.09× faster), restoring the earlier performance range. Exact tests,
controls and limitations are below. Development is stopped after this bounded request; wave work
and the legacy OptiX8 HitObject decision remain parked.

The prior three requested items—fresh validation, ordinary-IR bitfield cleanup and compilation/GPU
performance refresh—are complete. Their broad performance observations retain their measured
compiler identities; this fix refreshes material performance and focused compilation costs only.

The CUDA-text route migration is **complete with focused local acceptance**. NVVM no longer
infers operations from CUDA assembly-body strings or active semantic tags. The final field-offset
recognizer is replaced by a typed query that preserves the exact field key before optimization.
Explicit LLVM/libdevice names and genuine primitive PTX remain intentional backend interfaces.
Module43, ABI46 and container2 are unchanged. The signed16 O3 failure is corrected by provider-side
normalization at exact-width integer consumers. The consolidated integration checkpoint is accepted;
operation dispatch, shared type-role admission, structured-buffer planning, fake-provider maintenance
and architecture refresh are accepted. Native Half ceil/floor/trunc and single-rounded FMA are
accepted. The requested full validation checkpoint is accepted. Subsequent feature work is recorded
below; the earlier non-Torch SlangPy failures are resolved and development is stopped.

The callable family and requested RHI/working integration are complete. Explicit OptiX8.0/8.1/9.0
targeting and common raygen/trace/callable qualification are now accepted. Application-led feature
development remains stopped; the unresolved Torch families are recorded below. **The legacy HitObject compatibility decision remains parked:** native8.x
Invoke loses ray flags that9 retains;
preserving modern visibility without reducing the32-word payload capacity needs a private
cross-stage context ABI. No older HitObject implementation is enabled. Use eight-job builds and
economical focused validation. No push, external messages or system changes.
Preserve the user's untracked `tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw full evidence is under ignored
`build/nvvm-integration/full-after-cleanup-1/`; Half evidence remains under
`build/nvvm-half-native/` and earlier cleanup evidence retains its recorded paths.
Plans and reports remain uncommitted.

## Falcor2 scalar-condition vector selection

The standalone [shader](../tests/cuda/applications/falcor-scalar-vector-select.slang) preserves the
original failure from `compute_triangle_max_emission`, called by `EmissiveGeometrySystem::update`.
Its surface-interaction helper reaches this code in Falcor's `triangle_geometry.slang`:

```slang
let is_front_face_cw = false;
let N = normalize(cross(v1.position - v0.position, v2.position - v0.position));
let normal_os = select(is_front_face_cw, -N, N);
```

Both original debugger captures showed `select(false, float3, float3) -> float3`. This is canonical
IR from `core.meta.slang`'s `select<T>(bool, T, T)`. The shared semantic catalog previously required
predicate/result lane equality, rejecting one Bool lane versus three Float lanes before provider
emission. It now uses `hasComponentWiseLanes`: either one predicate lane or the result width.
Boolean predicate kind, admitted result types and exact alternatives remain checked. Existing
LLVM `CreateSelect` emission is unchanged. The previous scalar-Bool/int2 negative unit is now
positive; mismatched-width Boolean masks, numeric masks and mismatched alternatives remain rejected.

The initial replay had five passes and four E52017 rejections. The fresh replay passes all nine
cells, including constant and dynamic scalar predicates and the vector-mask control at NVVM O0/O3
and NVRTC O3. All nine have authored compile regressions. The separate
[runtime fixture](../tests/cuda/nvvm-scalar-vector-select.slang) executes the admitted vector families
with independent expected outputs and both predicate values. Independent review approved the
single shared-rule change and coverage. No new runtime working-tier configuration is admitted here.

Falcor's original select issue is resolved, but subsequent `accumulate_triangle_emission` compilation
rejects a 64-bit vector wave sum. That separate operation remains unimplemented. Exact current
identities, preserved failure histories and checks are in [the corpus manifest](application-corpus.manifest.json)
and [application status](falcor2-status.json). Raw final evidence is under
`build/nvvm-falcor-select-fix/`; original captures remain under `build/nvvm-falcor2-reduction/`.

## Current validation and maintenance

The compact-group cleanup adds nine exact working admissions: three existing compact-float3 modes
and six mixed-record default/Scalar/CData modes. All 1,726 earlier working outcomes and 569 earlier
units remain passing, with no changed campaign inputs. New focused controls pass 15/15, existing
snapshot/storage/borrowed controls 45/45, selected RHI 4/4 with 55,633 assertions, and the AD guard.
The static contracts pass after correcting one inherited Half2 rejection assertion: numeric Storage
was already admitted by earlier application work. Runtime compiler/provider/source pins remained
unchanged through the full campaign; the later static-only correction has its own test/binary pins.
The full RHI and SlangPy suites were not rerun. Exact outcomes and failed development attempts are in
[focused evidence](focused-evidence.json), under `build/nvvm-compact-conversion-cleanup/`.

The fresh initial checkpoint passes **1,724/1,724 working configurations** and **569/569 NVVM units**
with unchanged inputs. The full RHI run then exposed two regressions: provider-created dynamic array
snapshots lost Boolean vector lanes, and CUDA heap destruction leaked native pages while a shared
context remained alive. Both are repaired at their producers/owners, with failing-before controls.

The repaired fresh full NVVM RHI checkpoint is **280 passes, 1 intentional unsupported MakeHit failure
and 10 skips across all 291 registrations**. All 72,074,710 successful assertions are retained;
the two failed assertions belong to MakeHit. The earlier wrong outputs, OOM and crash remain in the
[RHI manifest](rhi-cuda-status.json). Nested Boolean snapshots pass both NVVM modes and NVRTC; the
SlangPy differentiated-array control also passes. This does not rerun the full SlangPy acceptance.

The bitfield cleanup is complete: extract/insert lower to ordinary IR, and the custom planner and
emitter family is removed. Dynamic boundary controls cover signed/unsigned 8/16/32/64-bit scalars
and vectors, including empty and full-width operations. Final validation passes **569/569 units,
16/16 smoke and 1,726/1,726 working configurations** with unchanged source and binaries. All 1,724
previous working passes remain passing; two bitfield configurations are new. The fresh full RHI run
preserves the exact 280 pass / 1 intentional unsupported MakeHit / 10 skip outcomes.

Current evidence is under `build/nvvm-current-checkpoint/` and `build/nvvm-bitfield-legalization/`;
[focal records](focused-evidence.json) retain the initial failures, controls and final reconciliation.

The material snapshot fix passes **1,726/1,726 working configurations**, preserving all previous
passes with unchanged inputs, **569/569 units**, **16/16 smoke**, and the new static storage-load
regression. Reverting only the fix fails that regression's two ordinary-load checks; aligned/scoped
controls preserve their exact original memory operation. Focused execution passes **35/35**,
selected RHI passes **4/4 with 55,633 assertions**, and the differentiated-array AD control passes.
This does not claim a new full RHI or SlangPy run. Exact source/binary pins remain unchanged through
the final campaign in `build/nvvm-material-promotion/`; module 43 / ABI 46 / container 2 are unchanged.

## Current performance and next priorities

The preceding full refresh on this L4 / driver 595.71.05 / eight-CPU host measured NVVM O3 compilation
as **1.95× faster on the 406-case frozen cohort and 1.82× on the 98-case discovery cohort** by geometric mean of per-case
median NVRTC/NVVM ratios. All 1,512 variants compile/assemble and all 9,072 measured samples plus
3,024 warmups preserve qualified PTX. Historical host and dependency changes prevent attributing
this larger relative advantage to an NVVM compiler speedup.

The material regression is resolved in the current focused qualification. An ordinary whole-context
storage copy before unpacking prevented scalar promotion. Using the existing recursive unpack helper
at the original read restores both entry frames from 592/624 bytes to zero, with zero spills. Eval
has zero static local stores; sample retains eight library stores. Value snapshots, compact pointer
layout and attributed memory operations remain intact. Inlining and matrix-snapshot controls did not
fix the regression; a late load-deferral experiment was discarded.

The current and requalified pre-fix NVVM O3 cubins use the same helper, oracle, inputs, GPU and driver.
At 1,048,577 records eval/sample improve **2.741/2.381 → 0.313/0.391 ms**. Current NVRTC O3 controls are
**2.815/2.488 ms**. Each arm passes 18 qualification cells, 216 measured launches and 72 warmups with
all active/guard bytes checked. The earlier confirmed 8.75×/6.08× slowdown against historical cubins
remains recorded as resolved history. These are periodic synthetic inputs with hot 2×2 textures,
not full renderer throughput.

Paired fresh-process compilation using preserved before/after compiler libraries gives material
medians **1.593 → 1.599 s** and **1.711 → 1.757 s** (+0.4%/+2.6%). Eight numeric variants range from
−6.3% to +1.1%; this small three-sample selection does not replace the full compile benchmark.
The initially longer RHI duration also occurs on the old compiler: matched compact-pointer tests
pass in 113.77 s before and 116.55 s after. The earlier approximately 82 s run is not a causal baseline.

The full original-input dispatch refresh preserves all 3,402 prior measured cells and adds 22:
**3,424 measured / 56 excluded**, 30,816 samples, 567 complete cases. The copyable-context fixture now
passes all six cells after explicit initialization. Of nine ratio-eligible cases, eight wave/min/max
fixtures take **2.07–3.03× NVRTC O3 intervals**; FP8 is near parity. Fresh 27-cell code analysis shows
missing integer-prefix tree fast paths and extra floating-fold mask/control work, with zero offline
spills. These are correctness workloads with shader checks and host enqueue gaps, not application
throughput. The full historical 580-case static analysis was not rerun.

The compact parameter-group conversion cleanup is complete. A useful next bounded audit is
checked array storage provenance: an inherited raw direct-IR conventional-array shape can lose its
storage role during element selection. It did not select the deleted decoder, and no source-level
regression is demonstrated. Prove its producer/admission contract before changing representations.
Wave fast paths remain a separate performance opportunity. Neither recommendation resumes work.
Current material evidence and a readable before/after report are under
`build/nvvm-material-promotion/presentation/report.md`; the preceding full compile/dispatch report
remains `build/nvvm-results/2026-10-03-current1/presentation/report.md`. The
[focused evidence](focused-evidence.json) records exact transitions and scopes.

## Torch findings and local application reproducers

The initial capture with PyTorch 2.8.0+cu128 and the native `slangpy-torch` 0.7.0 bridge installed
used the same 688-node
Torch-related selection: **626 pass / 11 compilation failures / 1 timeout / 50 authored skips**.
All 514 earlier passes remain passing. Of 113 bridge-blocked nodes, 110 now pass and three expose
native variants of the existing polynomial layout failure; two previously skipped bridge contracts
also pass. The previous eight compile failures and softplus timeout persist. All **12 exact problem
nodes passed with NVRTC**, with five route controls passing for each selected backend. These are
the pre-fix outcomes; the original node histories and binary identity remain in the manifest.

The [application manifest](application-corpus.manifest.json) maps all 12 nodes to four standalone
source reductions, preserving the failure shape and desired semantics. The bounded follow-up fixes
CUDAKernel roles in `fixEntryPointCallsites` and explicit parameter-group pointer provenance in
`_planNVVMLoad`, reusing existing cloning and address-space conversion. All five affected application
nodes execute correctly, and all 49 previously passing neighbors in those three files remain passing.
The native bridge and explicit NVVM routing are verified. Polynomial/softplus application nodes were
not rerun. Current SM80/CUDA12.9 source replay uses the repaired compiler, without package dependencies:

| Local entry / mapped application nodes | NVRTC O3 | NVVM O0 | NVVM O3 |
| --- | --- | --- | --- |
| CUDAKernel helper call / 3 | Pass | Pass | Pass |
| Packed tensor / 2 | Pass | Pass | Pass |
| Polynomial out backward / 6 | Pass | E52017 `local helper-value layout` | Same |
| Softplus tensor backward / 1 | Pass | Pass | 60-second timeout |
| Ordinary helper control | Pass | Pass | Pass |
| Scalar softplus backward control | Pass | Pass | Pass |

All local corpus passes include PTX assembly, **not GPU execution**: 15 pass, two layout failures and one
timeout across 18 cells. Softplus now has a standalone O3 compilation timeout; its native stalled
phase and identity with the application stall remain unproven. The packed reduction retains the
slicing overload selected with `SliceD == D`; replacing it with a descriptor copy changed the original
diagnostic. The two remaining diagnostic families do not establish root causes. Each reduction represents a family,
not every rank, bridge mode or differentiation variant in its application mapping.

Initial application evidence is `build/nvvm-torch-native-status/`; original captures/reductions and
before-fix replay remain under `build/nvvm-application-reproducers/`. Current application comparisons,
loaded identities, local replay, units and smoke are under `build/nvvm-torch-easy/`. The manifest
preserves the resolved diagnostic histories and exact source/binary pins. Kernel/packed sources now
also have authored compile regressions; the other two remain opt-in exploratory inputs. The separate
non-inlined kernel control executes on NVRTC O3 and NVVM O0/O3 and checks the untouched tail. No
working/smoke admission or broad-suite refresh is claimed. Independent review found no blocking defects.

## Prior application acceptance — non-Torch SlangPy failures resolved

The full CUDA-selected SlangPy checkpoint is **1,591 passes, zero failures, 807 skips and
3 expected failures** across **2,401 nodes**, plus 14 module-level skips. All **29 previous failures
resolve**, all **1,561 previous passes remain passing**, and no node was removed or demoted.
One new RGB32Uint metadata test accounts for the extra node. Exact node comparisons, intermediate
failures and executable identities remain in [SlangPy status](slangpy-cuda-status.json).

The accepted families cover pointer/helper qualification and generic symbol identity; fixed resource
arrays and nested acceleration-handle/parameter-group values; complete standard numeric CUDA uniform
storage; unused type-only declarations; and immutable AD field/array/vector/matrix updates. Dynamic
array reads use checked local snapshots to avoid the captured CUDA12.9 libNVVM optimizer stall.
The five RGB failures were metadata-only fixture allocations: tests now use the production texture
type factory without allocating unsupported physical RGB CUDA textures. Live texture tests still run.

Permanent runtime controls pass at NVVM O0/O3, with NVRTC/CPU controls as appropriate. The high-bit
array-index fixture reaches LLVM as `i8` and zero-extends before addressing lane 200. The full NVVM
unit checkpoint passed **564/568**; all four test-only failures pass in an exact repaired selection.
These repaired stale compact-uniform rejection/fake-type assumptions, an old select-chain assertion,
and inherited surface recorder count typos. No production workaround was added for these failures.
The working corpus measured **1,714 passes and 2 preflight failures**; both exact SM80 failures
now pass after repairing readonly uniform forwarding and the producer's shared physical-type cache.
The runner also recorded a source-edit warning: the repair was edited during testing, but no rebuild
occurred until the full run completed. This is full observed output plus focused repairs, not a fresh
all-green working run. The final **16/16 smoke** and **5/5 readonly/layout controls** pass. Eight new
runtime configurations join the working inventory from focused evidence, bringing it to 1,724.

An additional exploratory source using an explicit free `__constref` call with a pointer-bearing
constant buffer crashes in shared typeflow. It also crashes with the preserved upstream compiler;
the equivalent readonly method produces the intended NVVM rejection and is covered before mutation.
This inherited issue is separate from the now-passing SlangPy suite and remains recorded for future work.

SlangPy's full pre-commit checks pass. Current raw evidence is `build/nvvm-application-values/`;
[focused evidence](focused-evidence.json) records the final smoke/working checkpoints and reviews.

**Stopped as requested:** all failures in that non-Torch checkpoint are resolved. Do not resume the general
feature loop without a new instruction. The legacy OptiX8 HitObject decision remains parked.
Torch was outside that checkpoint; its later focused results are recorded above. Other SlangPy
platforms and physical RGB CUDA textures remain outside this qualification.

## Prior TensorView and structured-storage checkpoint

The TensorView/structured-storage slice is complete. TensorView and DiffTensorView use a typed,
host-compatible descriptor and ordinary core composition for queries, indexing, load/store,
references and public atomics. Scalar ranks 1–5, vector widths 1–4 and noncontiguous strides are
qualified. Exact reference-accessor returns preserve addresses; checked generic/shared/global atomics
retain local-storage rejection. Default structured buffers share canonical CUDA numeric storage
with ordinary pointers, preserving Bool bytes, width-3 packing, matrix orientation and aliases.
Module 43, ABI 46 and container 2 remain unchanged.

A fresh full CUDA-selected SlangPy run has **1,561 passes, 29 failures, 807 skips and 3 expected
failures** across the same 2,400 nodes, plus 14 module-level skips. This slice resolves **30 failures**,
preserves all **1,531 prior passes** and introduces **zero regressions**. The matrix-return regression
found during validation is repaired in the nested buffer type-declaration closure. The original full
1,054/536 checkpoint and intermediate failures remain in [SlangPy status](slangpy-cuda-status.json).

The **1,716-cell working checkpoint passed 1,713 and found 3 crashes**, all in the differentiable
scalar-to-shaped-value test. Serialized numerics modules predated the new IR operations; rebuilding
the existing standard-module targets repairs all three exact configurations without a compiler
workaround. The affected numerics suite also passes 59 tests, with one ignored. This is a full run
plus focused repairs, with zero changed inputs, not a fresh all-green full run. All **16 smoke cells
pass**. This checkpoint plus exact repairs owns the new cadence baseline. The initial full NVVM unit
run passed 548/565; all 17 failures are repaired in a 19-case focused selection, with one additional
nested matrix regression unit passing. These are full-unit results plus exact repairs.

GPU qualification includes 204 Tensor numeric cells and 204 structured numeric cells at O0/O3, both
matrix orders, independent byte guards, Bool 0x80 and Half3 padding. Three Tensor registrations pass
21,633 assertions, including the 16-cell public atomic family; structured storage passes 18,121
assertions. Twelve source/unit controls pass, including CPU/NVRTC/NVVM reference returns, invalid-return
diagnostics, shared floating/nested-resource atomics and preflight no-mutation boundaries. After the
final CUDA reference-template repair, the NVVM descriptor and all three general reference-return
controls pass again. Exact build identities, failed attempts and acceptance are in
[focused evidence](focused-evidence.json); raw evidence is under `build/nvvm-tensor-views/`.
No full RHI/native/surface/material rerun is claimed.

RHI now has 290 registrations (289 unique): mixed-age NVVM **279 pass / 1 fail / 10 skip**; NVRTC
**271 pass / 7 fail / 10 skip / 2 unrun**. Four new NVVM registrations pass. The selected NVRTC atomic
comparison passes. Its descriptor comparison initially failed to compile because reference templates
returned values where pointer results were required; all eight templates now return the existing
lvalue's address. The retry passes 1,135 assertions and fails only the two O0/O3 wide-stride checks:
CUDA wraps the product to `0x1c`, while NVVM produces the intended 64-bit `0x10000001c`. Both numeric
NVRTC registrations remain unrun. Existing MakeHit and comparison failures retain their histories in
[RHI status](rhi-cuda-status.json).

At that checkpoint, Torch MakeTensorView was unavailable/unqualified; no new lifetime or bounds
analysis is claimed. Direct immutable global aggregate constref forwarding remains rejected. Current
shared StructuredBuffer lowering can retain stale reads through a writable alias; qualified mutable
alias observations use two RWStructuredBuffer views. The failed exploratory alias attempt is retained.

The later application acceptance above resolves this checkpoint's remaining 29 failures. Its exact
first diagnostics and original outcomes remain preserved; the OptiX8 HitObject decision stays separate.

## SlangPy application checkpoint

The requested on-demand CUDA-selected SlangPy checkpoint is complete: **1,054 passed,
536 failed, 807 skipped and 3 expected failures across 2,400 test nodes**, plus 14 module-level
collection skips (12 Torch-related, imgui_bundle and tev). No crashes, collection errors or unrun
collected nodes remain. The corrected full run took 95 seconds. CUDA selection excludes functions
without a `device_type` parameter; this is not qualification of every SlangPy platform or Torch.

The isolated SlangPy build uses local Slang/RHI, eight build jobs, and explicit typed NVVM/SM80/
actual OptiX options in every CUDA session. Five route/selector tests pass; helper, direct and
custom-session GPU outputs distinguish NVVM from NVRTC. The original source-package extension
and build marker remain unchanged. LFS images/reference data are hydrated and preserved at the
maintainer's request. SlangPy pre-commit passes. No compiler implementation was changed here.

[slangpy-cuda-status.json](slangpy-cuda-status.json) retains exact failures, skips, comparisons,
loaded binary hashes and initial failure histories. The first NVRTC selection passed 32 of 35;
shared image failures were unresolved LFS pointers and a texture-format restriction. After setup
repair, all 13 final targeted controls pass. The original full run's six asset failures are resolved;
the order-sensitive command-buffer cleanup assertion is retained separately from its isolated
entry-parameter rejection. Do not infer full NVRTC acceptance from representative comparisons.

The first two priorities are now accepted: fixed numeric aggregate entry ABI and byval numeric
record forwarding. One recursive CUDA layout plan decodes launch storage into canonical values;
nested Half records also use the established integer helper transport at all four call boundaries.
The complete numeric fixture passes at O0/O3 on NVVM and NVRTC (2,089 assertions per route).
Fifteen focused units, four static checks and all 16 smoke cells pass. Builds use eight jobs.

The affected SlangPy selection passes 37 and fails 390 of 427 nodes: **30 previous failures resolve**,
including all 12 value-call failures. Remaining selected diagnostics are 353 entry-parameter
rejections and 37 sequential-element-pointer rejections. These expose pointer/resource shapes
outside numeric admission. The maintained mixed-age inventory is **1,084 pass / 506 fail / 807 skip /
3 expected failures**; the full run above retains its original identity and counts. This was a focused
rerun, not a fresh full application checkpoint. Exact transitions and evidence are in the manifest.

Local resource helper transport is now accepted for existing buffer, texture, surface and sampler
leaves, records and fixed arrays. Value inputs/results and local out/inout/readonly references retain
checked parent provenance, exact pointee identity and access restrictions. Shared CPU/CUDA layout
now sizes byte-buffer descriptors as pointer plus count, including nested records and arrays.

Focused qualification passes 3 live buffer cells, 40 handle-transport cells (20 shapes at O0/O3),
8 units, 6 static checks and all 16 smoke cells. Synthetic handle tests qualify bit transport;
SlangPy supplies separate live 2D/3D sampling evidence. The affected application selection is
**46 pass / 16 fail**, resolving **9 previous failures** and preserving all 37 prior passing controls.
At resource-family acceptance, the mixed-age inventory was **1,093 pass / 497 fail / 807 skip /
3 expected failures**.
The full checkpoint above is unchanged. Evidence is under `build/nvvm-resource-helpers/`.

The shared RHI cached-PTX repair is also accepted (`aec3e12ac` in sibling RHI). CUDA modules now own terminated text copied from
the exact artifact span; CUDA loading and OptiX inspection share that owner. The deterministic
cache fixture fails on both old routes and passes on both repaired routes, with raygen/triangle
controls (3 cases / 97 assertions per route). SlangPy now passes 47 of the same 62 selected nodes,
resolving its cache failure and preserving all 46 prior passes; NVRTC cache plus five route checks
also pass. Current mixed-age inventory is **1,094 pass / 496 fail / 807 skip / 3 expected failures**.
Compiler smoke evidence is reused unchanged; no full rerun. Raw evidence: `build/nvvm-cached-ptx/`.
The refreshed RHI inventory includes the prior numeric fixture and new cache fixture: 283 registrations
(282 unique), mixed-age NVVM272/1/10 and NVRTC268/5/10. Full checkpoint identities remain unchanged.

Pointer-bearing entry/helper transport is now qualified. One CUDA layout decoder handles finite
pointer-bearing launch records/arrays and whole parameter-group values; pointer leaves cross AS1
storage to AS0 executable values. Half call transport retains its own cache. Checked local child
references preserve readonly/mutable access. Global-context replacement now propagates address
spaces through nested arrays and fields. Typed raw offsets require proven CUDA/LLVM layout agreement.

The 452-node SlangPy selection is **263 pass / 189 fail**, resolving **216 old failures** and preserving
all 47 prior passing controls. Current mixed-age inventory is **1,310 pass / 280 fail / 807 skip /
3 expected failures**. The original full checkpoint is unchanged. Both-route O0/O3 ABI checks pass
155 assertions each,32 NVVM static tests and the address-propagation test pass, five CUDA/SPIR-V
address controls pass, and all 16 smoke cells pass. One stale Callable transform expectation was
corrected to the already-supported production contract. Raw evidence: `build/nvvm-pointer-entry/`.
RHI now has 284 registrations (283 unique), with mixed-age NVVM 273/1/10 and NVRTC 269/5/10.

The subsequent compact-pointer qualification is recorded in the current section above. The original combined
generic handle specialization duplicate-symbol failure remains recorded; the new canonical pointer
specialization regression passes, while that older combined fixture has not been rerun. No automatic demotion or full-suite freshness claim is made.

Open shared issue: column-major entry matrix reflection disagrees with CUDA's row-array physical
representation. Both routes fail 11 lanes per optimization in the retained mixed-matrix fixture;
explicit row-major non-square cases pass. Keep this separate from resource-buffer matrix layout.
Raw focused evidence is under `build/nvvm-entry-aggregate/`.
The legacy OptiX8 HitObject semantic decision remains parked independently of this backlog.

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
Subsequent implementations have passed focused review; [focused evidence](focused-evidence.json)
now contains 90 feature objects (including corpus tiers and OptiX raygen/triangle/material paths),
preserving all prior objects and the full baseline. Each feature's
identity owns its tested source/binaries; the configured compiler string still does not track HEAD.
These batches used eight-job builds and focused validation. The authorized sequence is complete:

1. Unify pointer-to-UInt64 reinterpretation with the existing checked address-observation contract.
2. Add explicit-layout record field loads and stores across Std430/Scalar/C in bounded batches.
3. Investigate and fix retained static-state dispatch and target-option relinking issues where
   the intended contract is established.
4. Investigate restricted CubeArray binding and mip-query failures, implementing fixes supported
   by the findings and retaining unresolved external/semantic boundaries.

**Resumed on 2026-10-01:** establish smoke/working/exploratory selection using existing runners,
then executable OptiX raygen, triangle hit/miss and representative material paths. Smoke runs after
implementation iterations; working runs every three to five iterations or sooner for broad changes.
Explore application-relevant failures in bounded batches and preserve intentional semantic differences.
The callable discussion stop was superseded by the explicit multi-version OptiX resume.
Approximate Half policies remain separate work.

The tier selector is implemented and reviewed: 1,713 working configurations, 459 exploratory
configurations and a 16-cell smoke subset. Its original 43 CPU contracts and 15 smoke cells passed;
the first smoke run took 22.16 seconds. The integration run below and reviewed focused admissions qualify this inventory.
The three descriptor-conversion input changes are reviewed: their stronger UInt2/guard oracle
preserves the original UInt64 checks, and all three current configurations pass.
OptiX raygen is accepted: O0/O3 each pass two changed launches with complete 170-word output,
guards, reflected ABI and no skips. Provider signature/no-mutation, compute-stage rejection,
static SBT stage/type/load checks and two PTX fixture cells pass. Initial test-only COM/CLI/output
issues and their focused retries remain recorded. The same build passes all 15 compute smoke cells.
OptiX triangle runtime now passes: UInt and Float4 at NVVM O0/O3, plus two NVRTC controls,
each check four hits, four misses and all 34 words with unequal barycentrics and untouched storage.
All 14 focused checks, three static admission checks and 15 compute smoke cells pass without
skips. Final smoke took 22.08 seconds; focused OptiX took 7.03 seconds. This slice is accepted. Core capability, fixture syntax, assertion scope and
attribute-bitcast failures are retained with their corrections; no failure was demoted or skipped.
The textured MaterialX dielectric slice is now accepted: eight exact Float32 world-ray queries,
Miss/ClosestHit-only admission, and a live texture/BSDF/payload path. All seven focused checks and
15 smoke cells pass (9.53s and 21.84s respectively). All three modes pass the independent 202-word
host oracle. NVVM O0 differs from CUDA in 12 material words within the retained numerical budget;
O3 matches every word for these finite inputs. The initial sampler warning-only failure is retained.
No full generated-material, packed texture-handle or performance claim is made.
The working-corpus integration is qualified with focused repairs: the 1,708-cell run took
1,057.09s and returned 1,706 correct plus two BF16 local-record preflight regressions. An earlier
storage-role tightening omitted the existing BF3/BF4 local-record family. Restoring that one
Storage admission passes both exact discovery retries, the existing emission unit, a new
role/cache boundary unit and all 15 smoke cells (22.18s). The original 1,706 passes retain their
tested identity; only the two retries and smoke are fresh with the repair. Initial failures and
input-change histories are preserved; this is not a new full multi-suite baseline.
Active plans remain uncommitted.

Current additions beyond the full checkpoint are:

- Texture handles accept selected UInt2 low/high word transport. Non-mip dimensions accept
  int/uint/float results, including actual 1DArray/2DArray layer counts. Explicit full/restricted
  views are qualified; CUDA's zero layer counts remain a comparison difference.
- Local record arrays support internal mutable and readonly references, nested forwarding and
  canonical OutParam returns. Readonly transport preserves mutation visibility without granting
  writes; native array/pointer results and unproven roots remain excluded.
- Surfaces support in-range dynamic component stores, native32 1DArray/2DArray with correct
  layer-first provider calls, Half arrays/volumes, and explicit signed/unsigned 8/16 storage with
  matching logical 32-bit values across 1D/2D/1DArray/2DArray/3D, widths 1/2/4. Narrow integer stores
  retain the D3D-based saturation policy; Vulkan out-of-range write equivalence is unqualified.
  Native logical signed/unsigned 8/16 values now use matching inferred or explicit integer formats
  across the same geometries and widths, preserving native bits without narrowing conversions.
- Coherent pointer memory supports naturally aligned Int/UInt32/64 for exactly Device/global
  and Workgroup/shared with scoped relaxed operations. Actual SM70 admission, canonical memory
  attributes and checked producer spaces remain required; qualification is at SM80.
- Std430/Scalar/C Device record pointers support entry/internal-helper transport and
  checked direct fields of conventional constant buffers, signed32 offsets and explicit/reinterpret
  UInt64 address observation. Selected nested record fields now support load/store recipes with
  shared offsets and exact scalar payload widths; the motivating record has64/48/40-byte strides.
  Whole-record memory, exports, pointer results/general storage, scoped fields, inverse reconstruction
  and AnyValue expansion remain excluded. CUDA rejects Std430 and uses 40 for
  Scalar/C; its three nonzero Scalar mismatches remain recorded.

Items 1–3 are committed as `180094a4f`, `a4bb02c78` and `37675d632`: checked pointer
reinterpretation, planned explicit-layout field memory, independent linked-option/cache ownership,
and explicit initialization of the repeated-dispatch fixture. The latter passes six focused checks
and three launches in each of NVRTC O3/NVVM O0/NVVM O3. Historical failures remain retained.

The retained texture investigation is `features.nvvm-texture-dimension-investigation`, with raw evidence under
`build/nvvm-texture-dimension-investigation/`. Restricted CubeArray faces 6..23 select the intended
three source cubes but report 18 as depth through both runtime and direct driver creation. Mip-width
queries execute correctly through PTX JIT/cubin for full and restricted views. Mip-count/array-size
queries fail with error 500 at lookup (default loading) or load (eager), despite successful assembly and present
kernel symbols. The extra undefined descriptor-size symbol is a diagnostic lead, not a fix.
No complete new public texture contract is established, so item 4 changes documentation/evidence only.

The full checkpoint remains unchanged: its 36 main gaps and 249 surface cells retain their original
identities. Focused records supersede specifically resolved texture-handle, dynamic-surface and
coherent-groupshared cells. The surface harness currently has 121 rows; 38 new grouped rows (114 cells)
need reviewed adoption at the next deliberately selected full checkpoint. No automatic full run.

Keep these open distinctions visible:

- CubeArray endpoints 1..3 reported count 3 with shifted faces; face-aligned 6..23 now samples the
  intended cubes but reports 18. Driver creation reproduces the mismatch. Mip-count materialization
  failures and the full dimensions fixture remain unresolved. Do not divide counts or fabricate
  reserved descriptor metadata.
- The original three-layout pointer fixture now compiles through the shared checked pointer-to-UInt64
  conversion. Its unspecified pointer bindings are never executed, so this is a compile-only
  resolution. Broader conventional pointer storage and AnyValue roles remain separate work.
- Original physical-storage-buffer and redundant-coherent-load fixtures contain races or unsupported
  scopes. Their compile evidence does not resolve runtime failures. Graphics tests, hardware atomics,
  generic `requirePrelude` text and non-square host packing remain distinct from missing primitives.
- Unannotated mismatched bindings, packed formats and three-channel transfers remain outside the
  qualified surface contract. Dynamic component stores are non-atomic whole-texel RMW with in-range
  selectors only. Approximate Half exp2/tanh and round ties retain their separate documented policies.

The optional sibling RHI workflow is qualified in `build/nvvm-rhi-cuda/rhi-build`, linked to this
local Slang compiler/provider. The selector is committed as `318c2588` on sibling branch
`nvvm-cuda-workflow`; its existing `build-all` is unchanged. Four NVVM compute/raygen cases, five
NVRTC controls and a default compute case pass. Three malformed selectors reject; old headers
disable explicit selection and old-runtime probes reject unsupported flags. Library tracing verifies
the local compiler/provider. The initial CUDA listing had 262 registrations (261 unique names);
that setup did not run the full suite. The later complete application checkpoint is recorded below
and remains separate from automatic smoke/working cadence. Custom device/session paths and
internal direct NVRTC kernels retain their documented compiler owners.

Conventional-global Int32/UInt32/Float32 vectors of widths 2/3/4 are qualified through existing
checked storage plans. The canonical ScalarLayout field pointer retains its producer-owned key;
three-lane values occupy 12 bytes and use the existing compact-storage conversion. Six focused
runtime cells and two compiler units pass, along with the static plan/provenance test and all
15 smoke cells (21.75s). The new fixture's three modes are reviewed working-corpus admissions.
The vector-only RHI retry advanced to the missing `PrimitiveIndex` operation. That intermediate
failure remains recorded; the query batch below resolves it.

OptiX PrimitiveIndex, InstanceIndex and InstanceID now use exact UInt32 named calls with
closest-hit admission. The sibling RHI two-instance test passes its independent 30-word oracle
at NVVM O0/O3; the NVRTC control passes both levels. The original RHI triangle also passes:
its retained failure history is global uint2 field admission, then PrimitiveIndex, then success.
The grouped backend stage test covers nine new forbidden-stage cases before provider mutation.
A provider test's substring collision between instance_id and instance_idx required only exact
call-token matching; the initial failure and focused retry remain recorded. The retry, existing
raygen runtime and all 15 smoke cells pass (smoke 21.82s). No ABI/version change.

The next eight-case RHI probe passed six existing ray-state/identity cases and rejected HitKind
and RayFlags. Both now use typed UInt32 queries: HitKind in closest-hit, RayFlags in miss/closest-hit.
All nine final selected RHI cases pass (387 assertions), including the expanded two-instance oracle
at NVVM O0/O3. It checks all 44 words, both triangle faces and flags 0/1, including nonzero flags
in miss; the before-compiler NVRTC control passes the same oracle. The earlier 30-word identity
qualification retains its original source/binary identity. Both affected compiler units and all
15 smoke cells pass (22.05s). No storage or ABI expansion was needed.

The shared termination producer repair is accepted separately from stage support. Existing
KnownBuiltin metadata identifies IgnoreHit/AcceptHitAndEndSearch before redundancy elimination;
shared call effects preserve implicitly observed payload writes, while late hoisting requires a
known exit. CUDA text/name recognition is removed. The independent RHI nine-mode oracle initially
returned incoming7 instead of11/12/21/31 in12 terminating cases at O0/O3; all231 assertions now pass.
Ten source cells pass through eight initial passes and two retries after correcting only new
assertion ordering. The focused unknown-call/cycle static unit and 15 smoke cells pass (22.75s).
At that gate, NVVM final IR retained store11 and its register write before termination, then
rejected the unsupported AnyHit stage with E52017 and no PTX. Raw evidence is in build/nvvm-anyhit-producer.

The bounded AnyHit execution batch is accepted. Six exact Float32 object-ray queries and two
effectful Void termination calls reuse the existing provider ABI. Five NVVM RHI cases pass 418
assertions, including object origin/direction and the unchanged nine-mode termination oracle at
O0/O3. The strengthened affine oracle checks inverse-transformed, unnormalized values; its earlier
NVRTC control passes 129 assertions. Three before-NVVM cases failed O0 stage admission and never
reached O3. Two world-ray controls pass 64 assertions, both affected shared units and the recursive
source diagnostic pass, both static plan units pass, and all 15 smoke cells pass (21.97s).
NVVM recursion still rejects with E55214. At that gate, AnyHit attribute admission was statically
checked; the separate query batch below adds runtime observation. No ABI/module version changed.

The scheduled working checkpoint after five implementations passes all 1,711 configurations,
with no regressions or changed inputs on the compiler accepted as `0762f003c`, before the query
extension below. The historical full baseline retains its own identity. The full RHI suite had not run at that
checkpoint; its later application results are recorded below.
Raw evidence is under build/nvvm-anyhit; source and artifacts remained frozen throughout the run.

The AnyHit query extension is accepted: thirteen existing physical queries now pass the owning
stage policy. The independent sibling RHI callback oracle checks both unequal attributes as well,
with 289 assertions at NVVM O0/O3. Its prior-compiler NVRTC control also passes 289. The original
fixture produced twenty 1–4 ULP NVRTC barycentric mismatches; a reviewed change of direction Z from 1.5 to 2
kept exact expected values unchanged. Both original/retry identities and failures are retained.
The affected unit retains 62 forbidden-stage/no-mutation cases and passes; all 15 smoke cells pass
in 21.96s. The eight-job incremental build took 32.59s. No core/provider/static/API changes were
needed, so unchanged contracts retain their earlier evidence. Raw evidence: build/nvvm-anyhit-state.

Fixed-array payload admission is accepted through the existing register-packing traversal. Dense
positive literal arrays recursively total at most 32 words; explicit strides, padding and unsupported
leaves remain rejected. The independent flat Float32[12] and nested mixed 32-word RHI oracles pass
293 assertions at NVVM O0/O3, with matching prior-compiler NVRTC controls. All 23 static boundary
cases and 15 smoke cells pass (21.65s). A test-only static build typo was corrected with the existing
poison builder; production/runtime evidence is unchanged. Exact identities and retry history are
in focused evidence; raw evidence is under build/nvvm-optix-array-payload.

Pure module-expression placement is now qualified through the existing shared dependency-localization
pass at the final NVVM legalization boundary. The permanent
[fixture](../tests/cuda/nvvm-global-constant-expressions.slang) passes NVVM O0/O3 with all 30 words
checked (2.616s). The static dependency/no-mutation unit passes, including scalar and reordered-vector
Swizzle cases (6.935s); all 15 smoke cells pass (21.334s). Calls, loads, pointer/resource operations
and unknown effects remain outside the placement policy. The original source's global synthesized
record-constructor Calls remain unresolved; the qualified source constructs records locally. NVRTC
still rejects its dynamic device initializer and has no admitted runtime cell. Initial constructor,
NVRTC, Swizzle and CLI invocation failures retain their exact source identities in the raw evidence
under build/nvvm-global-expressions.

The original matrix raygen now compiles to PTX. All four complete RHI matrix cases advance to
E52017 for their exact CUDA-only ObjectToWorld/WorldToObject wrappers; no NVVM matrix runtime is
claimed. Their earlier NVRTC control passes four tests (162 assertions) on its recorded identity.
Five transform-list/instance-handle APIs are now qualified through exact typed SDK calls:
GetTransformListSize, GetTransformListHandle, GetTraversableTransformType,
GetTraversableInstanceId and GetTraversableChild. The dedicated sibling RHI oracle passes 165
assertions at NVVM O0/O3 (2.357s); its prior-compiler NVRTC control passes 165 (3.046s).
Two single-level static instances have distinct custom IDs. Both AnyHit and ClosestHit observe
53 guarded output words across two hits and a miss; the full 64-bit child is compared with the
host-created BLAS handle. Opaque instance handles are nonzero and stage-consistent, without a
fixed-bit expectation. PTX retains a dynamic `ld.const.u32` index feeding the list-handle call and
64-bit handle registers. Three affected units pass (14.111s), and all 15 smoke cells pass (22.236s).
The E41012 implicit optix_multilevel_traversal profile-upgrade warnings remain recorded without
functional failures. The original build log reaches 26/26, but its tool session lost the completion
record; a separate successful no-work retry verifies the build, not a 0.274s full-build duration.
Raw evidence is under build/nvvm-optix-transform-list; earlier failures and identities are preserved.

Instance-scoped matrix rows are accepted. The two public traversable matrix APIs return independently
checked forward/inverse coefficients for two asymmetric instances in ClosestHit: 215 RHI assertions
pass at O0/O3 (2.280s), with matching prior-compiler NVRTC controls. The real-provider unit passes
(1.417s); the 15-group static contract passes (7.057s), checking 24 valid plans and exact rejection
boundaries. Shared build 332.766s and static build 327.121s pass. Focused PTX inspection confirms six
row reads, and all 15 smoke cells pass (21.606s). Optional interface 8 preserves existing ABI46 tables
and rejects unsupported row use before module creation. Raw evidence: build/nvvm-optix-instance-matrix.

The scheduled working checkpoint after five implementations passes all 1,713 admitted configurations
in 1066.593s, with zero regressions and zero changed inputs on the matching instance-row compiler.
The earlier 1,711-cell checkpoint and historical full baseline retain their own identities. Current
inventory: 1,713 working / 459 exploratory /15 smoke. Sources and binaries stayed frozen for the run.

The requested complete RHI CUDA checkpoint has now run on local Slang `52509c728` and sibling RHI
`f48fee6e`. All 269 registrations (268 unique names, one duplicate preserved) have reconciled outcomes:

| Selection     | Passed | Failed | Runtime skipped | Wall time |
| ------------- | -----: | -----: | --------------: | --------: |
| NVVM          |    198 |     61 |              10 |   25.755s |
| NVRTC control |    259 |      0 |              10 |   89.312s |

All 61 NVVM failures are E52017 compiler rejections and pass the same NVRTC tests. These wall times
include different failure/execution outcomes and are not a performance comparison. Named outcomes
and assertions are reconciled; doctest's headline passed counts include runtime skips. There are no
interrupted or unrun registrations. Sources and binaries stayed unchanged. The maintained
[RHI test manifest and failure list](rhi-cuda-status.json) records every occurrence, exact initial
diagnostic, comparison, skip reason, compiler-route exception and unresolved family. Raw evidence:
`build/nvvm-rhi-full`. No RHI results were added to the Slang working corpus.

| Initial failure group                                                                                        |  Tests |
| ------------------------------------------------------------------------------------------------------------ | -----: |
| HitObject representation                                                                                     |     26 |
| Compute entry-point parameters                                                                               |     20 |
| Full active-list matrix composition                                                                          |      4 |
| Combined sampler helper values                                                                               |      3 |
| Trace payload boundary                                                                                       |      2 |
| Pointer-bearing record, bindless fetch, typed-buffer global, cluster query, callable ABI, intersection stage | 1 each |

These groups identify the first observed rejection, not necessarily the complete root cause.
The complete selected resource-parameter family is now accepted. All 24 original failures in its
scope pass, together with three new RHI fixtures: 27 cases / 105,767 assertions, no skips. The new
fixtures execute O0/O3 and check all 48 numeric entry shapes, buffer/view counts, texture/sampler/
combined handles and five writable texture geometries. Six static tests, focused provider/emitter
checks and all 15 smoke cells pass. Two existing fake-provider regressions were repaired and their
exact retries pass; production binaries stayed unchanged for that test-only repair.

Entry transport uses CUDA layout and separate carriers without changing ordinary helper/storage
roles. Equivalent structured views now convert byte extents to element counts at the conversion
boundary. Writable formats retain explicit declaration/descriptor ownership. Surface layer counts
use directly qualified CUDA height/depth queries: the separate `suq.array_size` probe rejects at
module load with CUDA801. The NVRTC surface fixture returns zero instead of layer counts 3/4 at
both O0/O3; its four wrong assertions remain recorded, with the allocation-based oracle unchanged.

The manifest preserves the original full run and all failed attempts. The accepted resource,
HitObject and current-matrix families resolved 54 original failures. The current data/query batch
resolves five more: sphere/LSS queries and payload layout, cluster ID, nested pointer records and
unused typed-buffer bindings. Current exact counts and comparison differences are maintained in
[rhi-cuda-status.json](rhi-cuda-status.json); focused updates are not another full-suite run.
The callable closeout below resolves the final supported original failure; only the agreed arbitrary
MakeHit exclusion remains.
Existing shaders/oracles were not weakened and no failure was demoted.

**HitObject feasibility gate:** both explicit SDK8.1 MakeHit constructors compile through
NVRTC, but the current OptiX9 module compiler rejects them with error7204: they require ABI102
or older, while the active OptiX ABI is105. The NOP control passes module/program/pipeline creation.
No launch occurred. This OptiX ABI is unrelated to Slang provider ABI46. Exact attempts are retained
under `build/nvvm-hitobject`; the first combined log interleaved stdout/stderr, so a same-binary
separated capture preserves machine-readable outcomes.

**HitObject lifecycle is accepted with focused validation.** The approved OptiX9 scope preserves
independent traced-hit/miss/nop objects through helpers, repeated invocation and SBT changes,
plus geometry queries, ordered transforms/object rays and all three reorder forms. All four
arbitrary MakeHit/MakeMotionHit constructors explicitly diagnose. Provider-private storage is 392
bytes with alignment 8 and all 31 transform handles owned; external entry/buffer/payload roles stay rejected.

The final 27-case RHI family passes 2,265 assertions, including the guarded 533-word lifecycle fixture
at O0/O3. Standalone nested instance/SRT/matrix motion passes 309 words at each level, with its exact
earlier identity retained; all four stages also compile at O0/O3 on the final compiler. Six static
units, six provider/stage units, six cross-backend constructor cells, four MakeHit negative cells,
eight motion compile cells and four attribute/empty-payload PTX cells pass. Five NVRTC controls and
all 15 smoke cells pass (22.00s). No new full checkpoint or working-corpus admission; this is the
second accepted implementation since the 1,713-cell working checkpoint.

RHI now uses the conservative OptiX ALLOW_ANY graph policy. Changing only that flag repaired zero
custom instance IDs and an LSS fault after saved-hit replay (70 isolated assertions). This is observed
SDK9/driver compatibility, with unmeasured specialization cost, not a universal API prohibition or a
claim of broader RHI motion/depth support. CUDA's lifecycle comparison still aborts module creation:
its object-ray getter uses an incoming intrinsic illegal in raygen. The independent oracle is unchanged.

The family also corrected canonical allocation effects and zero-word operation timing. Existing
AllocateOpaqueHandle is Void(destination); preserving its write effect fixes constructor initialization
without inventing opaque copies. NVVM payload packing now precedes empty-type cleanup: a retained
probe previously compiled but silently lost empty-payload Traverse/Invoke. Final PTX assertions
require both calls and every report widths 0..8. ReportHitOptix uses the same typed tuple path as
portable ReportHit. Exact failed builds, tests, context-poisoned outcomes and repairs remain in
[focused evidence](focused-evidence.json) and the [RHI manifest](rhi-cuda-status.json).

The current-ray matrix family is accepted: all four RHI ObjectToWorld/WorldToObject 3x4/4x3 cases
pass 162 assertions. A nested instance/SRT/matrix scene passes all 309 guarded words at NVVM O0/O3,
observing current matrices in AnyHit/ClosestHit during tracing and repeated invocation. Both provider
units and the expanded static row-contract unit pass, with no skips; smoke passes all 15 cells
(21.55s). Interface 10 reuses checked row descriptors and provider composition without exposing
SDK storage or confusing incoming and saved state. Intersection admission is statically checked;
its execution, changing-quaternion motion and maximum depth remain unqualified. Each row currently
composes the full list independently. This is the third accepted implementation since the working
checkpoint. Raw evidence and exact initial build/probe failures: `build/nvvm-current-transforms`.

The maintainer requested fixing the remaining RHI errors before returning to the ordinary feature
loop. Arbitrary MakeHit remains the approved OptiX9 exclusion. The data/query family now qualifies
padded 32-bit records/arrays/matrices and dense attributes, current sphere/LSS/cluster queries,
recursive pointer-record bindings and storage-only typed-buffer declarations. CUDA payload values
use logical matrix rows, separately from external buffer layout; existing vector legalization now
normalizes singleton rows after the matrix producer. RHI's typed-buffer setter no longer writes a
count across its eight-byte reflected slot. Actual typed-buffer operations and null dereferences
remain unsupported. NVRTC rejects the new singleton-matrix and 48-byte/eight-leaf attribute tests;
these comparison differences retain their exact oracles and failures.

The data/query batch is accepted with 12 RHI cases / 49,976 assertions, eight static tests,
seven provider/source cells and all 15 smoke cells (21.79s). The selected NVRTC comparison passes
10 cases and retains the two described failures. A revert drill restores the old RHI setter and
fails 32 guard-byte assertions; restoring the fix passes. Current mixed-age inventory is 278
registrations (277 unique): NVVM 266 pass / 2 fail / 10 skip; NVRTC 264 pass / 4 fail / 10 skip. The five new
fixtures stay in RHI, not the compiler corpus. Exact identities, attempts and independent review
are under `build/nvvm-rhi-data-families`; this is the fourth implementation since the 1,713-cell working checkpoint.
The callable family is now accepted: typed calls and callable device entries share the private
NVVM value ABI, including recursive numeric payloads, mutable out/inout, dynamic nested calls,
and recursively empty payload effects. The three RHI callable fixtures pass 1,046 assertions at
O0/O3. Static stage/signature checks, provider no-mutation checks and four source/PTX cells pass.
Module43/providerABI46/container2 remain unchanged. Cross-backend callable ABI and pointer/resource
payloads remain outside the qualified contract.

RHI reuses commit `81d5ded4` (Configure OptiX callable stack sizes) from the locally available remote
branch, with device graph-depth and deferred descriptor ownership adaptations. PR merge status was
not verified. The full suite executes 280 registrations (279 unique): **NVVM 269 pass / 1 fail /
10 skip**. All 60 supported original failures are resolved; arbitrary MakeHit is the remaining
agreed OptiX9 exclusion. NVRTC coverage is complete across an interrupted 158-case run and the
exact 122-case remainder: **265 pass / 5 fail / 10 skip**. Its retained differences are singleton
payload matrices, the new callable singleton-matrix fixture, HitObject lifecycle, padded attributes,
and surface dimensions. The lifecycle process abort and all unrun/remainder evidence remain visible.

The due working run executed all 1,713 configurations with zero input changes: 1,705 passed and eight
regressed (four resource/interface shaders at O0/O3). The previous data batch had applied a second
aggregate-layout proof to lowered resource/value carriers whose binding keys retain source layouts.
Exclusive routing through their existing physical record proof repairs all eight exact configurations;
storage-only aggregates retain their canonical layout checks. This is a full run plus focused repairs,
not a fresh all-green full run on the repaired compiler. The positive/negative preflight unit and
eight provider/source controls pass; 15 related RHI cases pass 51,020 assertions on the repair.
Smoke now includes append-buffer carrier coverage: all **16 cells pass in 22.05 seconds**.
Exact identities, original failures, retries and comparisons are maintained in
[focused evidence](focused-evidence.json), [RHI status](rhi-cuda-status.json) and ignored
`build/nvvm-optix-callable`. The original full baseline remains unchanged.

Explicit version targeting is accepted. `-optix-version 80000|80100|90000` and API option163
participate in existing target/link cache identity. RHI forwards the actual selected context SDK
and rejects conflicting session options; explicit NVRTC targets check their included SDK headers.
Optional provider interface12 configures empty modules, with the historical90000 fallback for
older providers. Module43/providerABI46/container2 remain unchanged. The common path qualifies
8.0/8.1/9.0; SDK 8 HitObject storage and operations remain rejected before emission.

Focused final results:16 compiler version/routing cells,13 existing stdout rejection controls,
NVVM9 pass/1 skip/1,401 assertions each on 8.0 and8.1,10 pass/1 skip/2,527 assertions on 9.0 (including the
retained lifecycle), and2NVRTC controls/24 assertions perversion. The runtime skip is the existing
entry-parameter write/cache test. All16 smoke cells pass in21.95 seconds, with no input changes. This is the
first accepted implementation since the working checkpoint. No new full-suite or legacy lifecycle
claim. The281-registration current RHI inventory has mixed-age SDK 9 outcomes: NVVM270pass/1 fail/
10 skip and NVRTC266pass/5fail/10 skip; the remaining failures retain their previous histories.

A new no-o rejection fixture uncovered an existing CLI producer bug: target options were copied
only while associating output files. PTX stdout could silently lose the NVVM selector and use
NVRTC. Moving the existing option merge to target creation fixes the source of truth, with no
emitter fallback. All13 existing no-o SIMPLE rejection fixtures were audited and requalified;
their historical passes alone did not establish NVVM routing. API/provider, explicit-output and
runtime evidence retains its own identity. Initial failures and exact final evidence remain in
[focused evidence](focused-evidence.json) and `build/nvvm-optix-versions`.

Native SDK probes on both8.x versions validate ordinary Trace flags and guards but observe zero
incoming RayFlags after both immediate Traverse/Invoke and explicit constructor/Invoke. SDK 9 retains
flags 1/2/8; with flag 8 its Invoke also suppresses CH, while8.x invokesCH. These are observed SDK
version differences, not a claim about Vulkan/D3D standaloneInvoke suppression. Retaining flags
beside an older snapshot would answer saved-object queries but would not repair RayFlags inside
invoked shaders. Retracing is not valid because it repeats traversal side effects.

**Parked legacy action: settle the pending8.x HitObject contract before implementing that
family.** The recommended next step preserves modern flag visibility and all32 payload words by
investigating a private cross-stage context ABI. The alternative explicitly accepts and documents
native 8 Invoke differences. Attribute-budget ownership and full topology reconstruction also require
qualification; there is no hidden downgrade. This decision boundary applies only to the parked legacy family.
Immutable global aggregate initialization and exploratory ranking remain queued and unstarted.
Custom RHI devices/sessions and internal NVRTC kernels retain their documented compiler ownership.
