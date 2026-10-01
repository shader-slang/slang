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
Seventeen feature implementations have since passed focused review; [focused evidence](focused-evidence.json)
now contains 59 feature objects, preserving all prior objects and the full baseline. Each feature's
identity owns its tested source/binaries; the configured compiler string still does not track HEAD.
Continue bounded reviewed work with eight-job builds and economical focused validation.

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
- Std430/Scalar/C Device record pointers support address-only entry and internal-helper parameters,
  signed32 offsets and UInt64 address observation. Strides follow shared layout rules; the
  motivating record has 64/48/40-byte strides. No dereference, export, pointer result/storage,
  inverse reconstruction or AnyValue expansion is implied. CUDA rejects Std430 and uses 40 for
  Scalar/C; its three nonzero Scalar mismatches remain recorded.

The latest batch is `features.nvvm-native-narrow-surfaces`, raw evidence under
`build/nvvm-native-narrow-surfaces/`. All thirty new NVVM O0/O3 physical cells pass. Across the
51 selected cells there are 39 passes and 12 retained CUDA compile failures: five new CUDA whole
rows pass, ten component rows fail unchanged, and six prior controls retain exact outcomes.
Two focused units, four negative cells, 121 harness contracts and 22 report contracts pass.
Ten existing PTX outputs contain the expected raw 8/16-bit scalar/vector surface instructions.
The eight-job build took 26 seconds. Provider primitives, provider binary and canonical surface
legalizer are unchanged, so their accepted primitive tests are reused. No production repair or
retry was needed. Representative high-bit loads, independent native marker stores and full guards
are qualified; exhaustive narrow decoding is not claimed.

The full checkpoint remains unchanged: its 36 main gaps and 249 surface cells retain their original
identities. Focused records supersede specifically resolved texture-handle, dynamic-surface and
coherent-groupshared cells. The surface harness currently has 121 rows; 38 new grouped rows (114 cells)
need reviewed adoption at the next deliberately selected full checkpoint. No automatic full run.

Keep these open distinctions visible:

- CubeArray restricted views reported a plausible count but selected faces shifted by one face;
  full-view success does not qualify restricted binding. Mip-query linkage failures and the full
  dimensions corpus fixture remain unresolved. Do not divide counts or guess view endpoints.
- The original three-layout pointer fixture now reaches an unsupported parameter-buffer field
  address after helper admission. Its unspecified pointer bindings are never executed. Broader
  conventional pointer storage and AnyValue roles require separate work.
- Original physical-storage-buffer and redundant-coherent-load fixtures contain races or unsupported
  scopes. Their compile evidence does not resolve runtime failures. Graphics tests, hardware atomics,
  generic `requirePrelude` text and non-square host packing remain distinct from missing primitives.
- Unannotated packed bindings, normalized formats and three-channel transfers remain outside the
  qualified surface contract. Dynamic component stores are non-atomic whole-texel RMW with in-range
  selectors only. Approximate Half exp2/tanh and round ties retain their separate documented policies.

Next: investigate checked parameter-group loads of address-only Std430/Scalar/C record pointers.
Freeze IR, reflection and an allocated binding fixture before implementation. Admission must belong
to parameter-group storage and a checked load plan; arbitrary loaded pointers must not acquire an
assumed global root. Keep general pointer storage, dereferences, pointer results, inverse casts and
AnyValue outside this bounded step. The original pointer fixture remains unresolved until its
specific contracts are independently demonstrated.

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
