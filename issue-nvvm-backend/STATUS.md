# NVVM current status

The CUDA-text route migration is **complete with focused local acceptance**. NVVM no longer
infers operations from CUDA assembly-body strings or active semantic tags. The final field-offset
recognizer is replaced by a typed query that preserves the exact field key before optimization.
Explicit LLVM/libdevice names and genuine primitive PTX remain intentional backend interfaces.
Module43, ABI46 and container2 are unchanged. The signed16 O3 failure is corrected by provider-side
normalization at exact-width integer consumers. The consolidated integration checkpoint is accepted;
operation dispatch, shared type-role admission, structured-buffer planning, fake-provider maintenance
and architecture refresh are accepted. Native Half ceil/floor/trunc and single-rounded FMA are
accepted. The requested full validation checkpoint is accepted. Subsequent feature work is recorded
below; development has resumed with corpus tiers and an initial OptiX path.

The maintainer approved corpus tiers and initial OptiX support on 2026-10-01 and explicitly resumed
development. Earlier stopping conditions are superseded. Use eight-job builds, related feature
batches, focused validation and reviewed local commits. Continue until a concrete blocker requires
human input or a new stopping instruction arrives. No push, external messages or system changes.
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
Thirty feature implementations have since passed focused review; [focused evidence](focused-evidence.json)
now contains 76 feature objects (including corpus tiers and OptiX raygen/triangle/material paths),
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
The previous four-item stop is superseded. Approximate Half policies remain separate work.

The tier selector is implemented and reviewed: 1,711 working configurations, 455 exploratory
configurations and a 15-cell smoke subset. Its 43 CPU contracts and all 15 smoke cells pass;
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
- Unannotated packed bindings, normalized formats and three-channel transfers remain outside the
  qualified surface contract. Dynamic component stores are non-atomic whole-texel RMW with in-range
  selectors only. Approximate Half exp2/tanh and round ties retain their separate documented policies.

The optional sibling RHI workflow is qualified in `build/nvvm-rhi-cuda/rhi-build`, linked to this
local Slang compiler/provider. The selector is committed as `318c2588` on sibling branch
`nvvm-cuda-workflow`; its existing `build-all` is unchanged. Four NVVM compute/raygen cases, five
NVRTC controls and a default compute case pass. Three malformed selectors reject; old headers
disable explicit selection and old-runtime probes reject unsupported flags. Library tracing verifies
the local compiler/provider. The initial CUDA listing had 262 registrations (261 unique names); the full suite has **not** run
and remains on demand outside automatic smoke/working cadence. Custom device/session paths and
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
extension below. The historical full baseline retains its own identity. The optional full RHI suite has not run.
Raw evidence is under build/nvvm-anyhit; source and artifacts remained frozen throughout the run.

The AnyHit query extension is accepted: thirteen existing physical queries now pass the owning
stage policy. The independent sibling RHI callback oracle checks both unequal attributes as well,
with 289 assertions at NVVM O0/O3. Its prior-compiler NVRTC control also passes 289. The original
fixture produced twenty 1–4 ULP NVRTC barycentric mismatches; a reviewed change of direction Z from 1.5 to 2
kept exact expected values unchanged. Both original/retry identities and failures are retained.
The affected unit retains 62 forbidden-stage/no-mutation cases and passes; all 15 smoke cells pass
in 21.96s. The eight-job incremental build took 32.59s. No core/provider/static/API changes were
needed, so unchanged contracts retain their earlier evidence. Raw evidence: build/nvvm-anyhit-state.

**Next work:** probe the four existing RHI ObjectToWorld/WorldToObject matrix cases and audit their
fixed-array payload and transform-composition boundaries. Raw SDK object-ray queries are AnyHit/
Intersection-only, so ClosestHit needs a different implementation, not broader stage admission.
GeometryIndex is not interchangeable with an SBT record index. Keep motion, procedural/callable ABI
and recursive callback tracing separate. This is one implementation iteration after the 1,711-cell working
checkpoint at 0762f003c; that checkpoint was not rerun or relabeled. Full RHI remains on demand.
