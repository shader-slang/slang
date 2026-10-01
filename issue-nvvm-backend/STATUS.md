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
Twenty-three feature implementations have since passed focused review; [focused evidence](focused-evidence.json)
now contains 67 feature objects (including corpus tiers and OptiX raygen/triangle tracing),
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

The tier selector is implemented and reviewed: 1,708 working configurations, 455 exploratory
configurations and a 15-cell smoke subset. Its 43 CPU contracts and all 15 smoke cells pass;
the first smoke run took 22.16 seconds. This inventory does not claim a fresh full working run.
Three working descriptor-conversion cells retain changed-input review status; none is in smoke.
OptiX raygen is accepted: O0/O3 each pass two changed launches with complete 170-word output,
guards, reflected ABI and no skips. Provider signature/no-mutation, compute-stage rejection,
static SBT stage/type/load checks and two PTX fixture cells pass. Initial test-only COM/CLI/output
issues and their focused retries remain recorded. The same build passes all 15 compute smoke cells.
OptiX triangle runtime now passes: UInt and Float4 at NVVM O0/O3, plus two NVRTC controls,
each check four hits, four misses and all 34 words with unequal barycentrics and untouched storage.
All 14 focused checks, three static admission checks and 15 compute smoke cells pass without
skips. Final smoke took 22.08 seconds; focused OptiX took 7.03 seconds. This slice is accepted. Core capability, fixture syntax, assertion scope and
attribute-bitcast failures are retained with their corrections; no failure was demoted or skipped.
The next bounded slice adds world-ray observations needed for textured MaterialX dielectric execution
in closest-hit. The prepared probe reuses existing material equations and the triangle harness;
it has not been executed yet. Active plans remain uncommitted.

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

**Active work:** qualify textured material execution and its required world-ray query batch. Corpus selection and raygen are complete; compute
smoke passed after each implementation. Run the working corpus at the next material integration
checkpoint (three feature iterations since introducing tiers). The prior full baseline remains
authoritative for its identity; this cadence does not request another full multi-suite campaign.

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
The earlier repeated-static fixture and link-option cache failures are resolved with focused
evidence; retain their original failure records.
CUDA `dim3 == uint3` source emission remains unsupported.
