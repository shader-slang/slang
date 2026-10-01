# Direct NVVM backend architecture

The experimental direct backend compiles linked Slang IR to PTX through libNVVM. It shares
semantic checking and common lowering with the CUDA C++/NVRTC route; NVRTC remains the default.
Correctness, compilation cost and generated-code quality are separate qualifications.

This document describes ownership and invariants. The [capability ledger](nvvm-backend-capability-ledger.md)
defines qualified combinations and exclusions. [STATUS](../../issue-nvvm-backend/STATUS.md) owns
current acceptance and work authority; [RESULTS](../../issue-nvvm-backend/RESULTS.md) owns validation
commands and maintained numerical contracts. [HISTORY](../../issue-nvvm-backend/HISTORY.md) and Git
provide recovery of superseded experiments and narratives.

## Pipeline and ownership

```text
Slang source → semantic checking → linked IR and shared lowering
    ├─ CUDA source legalization → CUDA C++ → NVRTC → PTX
    └─ NVVM legalization → preflight and owned plan → typed provider → libNVVM → PTX
```

The public target is `SLANG_PTX`. Target-scoped `-emit-cuda-via-nvrtc` and
`-emit-cuda-via-nvvm` select one canonical option; the last explicit selector wins. The direct
route uses an internal NVVM artifact, separate from the CPU LLVM target. When `linkWithOptions`
changes the route, the linked target program owns a copy of the effective target request before
capability computation. Layout, specialization and emission use that same request; other programs
and the shared session keep their original requests.

The `nvvm` capability refines `cuda`, so an explicit `case nvvm` wins over `case cuda`. CUDA device
and SM requirements remain applicable. CUDA source/header output and NVRTC PTX select `cuda`;
requesting a capability alone cannot override the selected emission route. The refinement still
inherits `textualTarget`, but no NVVM operation interprets a CUDA target-switch assembly template.

| Boundary                                                       | Owner                                                                                                        |
| -------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------ |
| Linking, specialization and shared pass order                  | [slang-emit.cpp](../../source/slang/slang-emit.cpp)                                                          |
| Core operation composition                                     | [hlsl.meta.slang](../../source/slang/hlsl.meta.slang) and the core modules                                   |
| Static surface formats and component updates                   | [slang-ir-nvvm-surface-legalize.cpp](../../source/slang/slang-ir-nvvm-surface-legalize.cpp)                  |
| NVVM-ready IR and legalization postconditions                  | [slang-ir-nvvm-legalize.cpp](../../source/slang/slang-ir-nvvm-legalize.cpp)                                  |
| Canonical type classification and role-specific representation | [slang-emit-nvvm-type-lowering.cpp](../../source/slang/slang-emit-nvvm-type-lowering.cpp)                    |
| Preflight, diagnostics and provider emission                   | [slang-emit-nvvm.cpp](../../source/slang/slang-emit-nvvm.cpp)                                                |
| Owned operation recipes and source index                       | [slang-emit-nvvm-plan.h](../../source/slang/slang-emit-nvvm-plan.h)                                          |
| Numeric descriptor admission                                   | [slang-nvvm-semantic-catalog.h](../../source/compiler-core/slang-nvvm-semantic-catalog.h)                    |
| Versioned provider boundary                                    | [slang-nvvm-ir-builder-api.h](../../source/compiler-core/slang-nvvm-ir-builder-api.h) and its builder facade |
| LLVM construction and NVVM adaptation                          | [slang-llvm-nvvm.cpp](../../source/slang-llvm-nvvm/slang-llvm-nvvm.cpp)                                      |
| Toolkit selection and vendor program lifecycle                 | [slang-nvvm-compiler.cpp](../../source/compiler-core/slang-nvvm-compiler.cpp)                                |

Shared producer defects belong before this boundary. For example, aggregate argument snapshots
must be established before deferred resource loads: emission cannot recover an earlier SSA value
by rereading storage after a write. Likewise, legal source spelling of a canonical `SwizzleSet`
belongs to the shared source emitter. These are not reasons to teach NVVM about accidental source
or IR representations.

`legalizeIRForNVVM` runs after linking and late bitcast normalization. It folds typed layout
queries, handles the selected derivative and bounds policies, removes the canonical read-none
`unmodified` check, performs cleanup and verifies postconditions. Zero-index bounds policy becomes
typed compare/select arithmetic using each access's own extent and index type; the direct route
does not preprocess the CUDA prelude. Selected local Boolean-vector lane writes become SSA lane
updates, without admitting escaping or external packed-lane references. Selected read-only texture
descriptor conversions between UInt2 and handles become unsigned low/high word operations around
the existing UInt64 handle conversion. The canonical descriptor resource type selects this lowering;
it does not grant integer conversion to buffer, sampler or writable-resource handles.

Typed layout queries retain the semantic fact that optimization could otherwise erase. `OffsetOf`
carries the exact canonical field key as part of IR identity, so equal-valued fields keep distinct
offsets. NVVM folds that key using CUDA layout and signed-Int32 range checks; other targets restore
the original call with single argument evaluation. Size/alignment use canonical layout types and
overflow checks. Legalization does not reconstruct field paths from optimized values, and arbitrary
GenericAsm, `RequirePrelude` or execution requirements are not general no-ops.

## Operation boundaries and compatibility

Core bodies own ordinary composition and explicit operands. The backend accepts canonical IR,
selected named interfaces, and the few primitives that require a provider implementation:

| Interface                                                          | Contract authority                                                                       |
| ------------------------------------------------------------------ | ---------------------------------------------------------------------------------------- |
| Canonical arithmetic, conversions, memory, atomics and resource IR | Compiler admission and typed provider descriptors                                        |
| Named `llvm.*` calls                                               | Explicit admitted intrinsic families; LLVM registry signatures, overloads and attributes |
| Named `__nv_*` calls                                               | Exact definitions in the selected immutable libdevice snapshot                           |
| Qualified primitive PTX                                            | Explicit provider recipe, target requirement and focused tests                           |

For example, `__intrinsic_asm "llvm.ctlz", value, false;` carries both checked operands in IR.
Helper parameters are not an implicit forwarding convention. The owned plan retains operand
values and type/constant-kind descriptors; each API call borrows a fresh descriptor view so moving
the plan cannot invalidate pointers into its old storage. LLVM's signature matcher determines
overload types, and `ImmArg` attributes require constants during the pure support query. Emission
separately validates actual values, types, constant promises, provenance, dominance and conflicting
symbols before creating a declaration or call. This does not admit arbitrary LLVM snippets or all
intrinsics registered by LLVM.

Libdevice signatures come from selected definitions, not a parallel production name table.
Pointer-output calls such as frexp/modf use OUT_POINTER descriptors carrying the pointee type.
Compiler preflight requires qualified writable local numeric pointers; the provider uses the same
typed AS0 pointer mapping for query and emission. LLVM registry calls do not inherit this role.
The [compute/value ledger](nvvm-backend-capability-ledger.md#compute-values-and-memory) records the
supported public operations, widths and pointer-output qualifications.

`resolveValueOperationFamily` is the single admission/diagnostic authority for numeric operation
descriptors. Its exact catalog retains five hardware-wave signatures: active mask, ballot and
three match payload types. Arithmetic, conversion and reinterpretation families do not have
duplicate exact catalog rows. Compiler and provider consume this authority; physical operand and
ownership checks still belong to emission.

Tests keep their model separate from those production authorities. Immutable source fixtures live
in [unit-test-nvvm-source-fixtures.h](../../tools/slang-unit-test/unit-test-nvvm-source-fixtures.h);
constants consumed by support-owned operation tables remain with those tables. The support header
retains one isolated fake state per translation unit. Its conventional libdevice signature table
is test-only, independent of public-operation expected-name assertions; real-provider tests also
bind unconventional signatures to familiar names to prove selected definitions remain authoritative.

Retired numeric identities remain reserved holes. They must not be renumbered, reused or accepted
through a fallback just because their source algorithms now live in core. The removed semantic-tag
extension has no active parser/lowering route; serialized AST token and stable IR slots remain
reserved so module43 layout is unchanged. Old tags/tokens diagnose rather than reviving the retired
semantics. Ordinary comma-separated intrinsic operands remain supported.

The prototype writes and accepts semantic **module43**, with **provider ABI46** and **container2**
as separate contracts. Older user modules and built-ins require recompilation for every backend.
Version guards reject incompatible serialized content before AST/IR decoding; metadata inspection
and speculative import fallback to source remain available. Direct retired-operation rejection is
independent of the module guard. See [module compatibility](backwards-compat-for-ir-modules.md#current-prototype-boundary).

## Preflight and the emission plan

Preflight validates reachable functions, helper/entry signatures, exact operations, operands,
layout, address relationships and target requirements before output-module or vendor-program
mutation. Unsupported forms return diagnostics. Important negative tests assert no output and
zero creation/mutation counters; a failed trial emission is not the support query.

`NVVMEmissionPlan` owns reachable function order, collision-checked physical names and source-keyed
operation recipes. `NVVMEmissionPlanIndex` enforces unique sources and supplies typed emission
lookups. Requirements are deduplicated by exact overload; emitted operations retain one record per
canonical source instruction. Querying a selected input library can allocate its isolated parsed
representation, but creates neither output IR nor a vendor program.

Ordinary allocation/load/store recipes retain the admitted role, alignment, conversion, load flags
and pointer-value ABI/provenance decisions. Field and indexed-address records retain canonical
field selection and qualified parent/storage/access facts. Diagnostic order and operand dominance
remain part of preflight; provisional records cannot reach emission incomplete. A read-only borrow
may reference mutable caller storage and does not itself authorize invariant-load metadata.

Structured-buffer loads retain the exact raw view, buffer/index, alignment, flags and conversion.
Raw roots and child field/index facts are recorded after parent/index availability checks; children
inherit canonical root identity and resource-view permissions independently of pointer spelling.
No recursive ancestor discovery is needed for this family. Preflight plans structured conversion
strategies and exact Boolean requirements together. Emission follows retained child indices,
preserving physical-array-struct identity and vector3 aggregate-storage/vector-value operations;
explicit-stride constructors share the same planned converter. Field-value extraction and other
dedicated resource operations remain outside this completed family. This is not a general
transforming physical-storage pass.

## Canonical types, storage roles and helper ABI

Canonical IR types, field keys and instructions remain the semantic source of truth. Equal physical
LLVM types do not imply equal Slang semantics: Half/BF16 both occupy 16 bits, and the two FP8 formats
both occupy 8 bits. `classifyNVVMType` provides provider-independent canonical analysis shared by
helper signature preflight and type lowering. `NVVMTypeInfo::supports` owns role distinctions.
Actual argument provenance, export restrictions and CUDA/LLVM layout compatibility remain separate
proofs.

The nine type uses are entry result, helper result, entry parameter, helper parameter, helper value,
ordinary value, local storage, parameter-group storage and structured-buffer storage. Support is
checked **before** cache access. Value/helper/storage representations have distinct caches; a helper
pointer key includes its pointee use. A successful storage lookup cannot authorize a previously
unsupported value, external reference or exported signature. Copyable HelperValue requests redirect
to Value before cache access, preserving request-order independence.

Half helper parameters/results use physical i16 scalars or `<N x i16>` vectors for admitted widths
2–4, while body values remain canonical Half. Callers encode arguments, callees decode parameters,
returns encode results and callers decode them, using bit-preserving reinterpretation.
`getNVVMHalfHelperABILaneCount` selects the same shapes at all four crossings. Preflight requires
both exact conversion directions before provider mutation. This avoids a qualified libNVVM O3
Half-vector call-transfer defect without changing arithmetic or storage admission.

Exported Half helpers retain the tested direct-NVVM PTX symbols and layout, including a half4-to-half3
boundary. The independent PTX caller checks semantic lanes and ignores unspecified return padding.
This is not CUDA-prelude binary interoperability: the captured NVRTC helper has different symbols
and packing. See [language composition evidence](nvvm-backend-capability-ledger.md#language-composition-and-application-evidence).

Address spaces are generic/code0, global1, shared3, constant4 and local5. `addrspacecast` preserves
provenance; integer round trips cannot substitute for it. Field selection uses the canonical field
key and qualified parent, not pointee type alone. Indexed children retain the parent's independent
access/storage proof. Constant-memory parameter-group loads remain ordinary loads; invariant global
buffer recipes require their separate immutable-location contract.

Natural payload layout, CUDA layout and physical LLVM layout are distinct. AnyValue packs Natural
payloads; internal copyable locals and borrows can use native value layout, including float3
alignment16. External storage and qualified BF16/compact families use their proven representations.
Layout caches distinguish rules, and aggregate checks verify member offsets, size and alignment.
Increasing allocation alignment cannot repair incorrect field offsets or array stride.

| Value                | Internal representation                | Qualified local storage / CUDA size and alignment |
| -------------------- | -------------------------------------- | ------------------------------------------------- |
| BF16 scalar          | i16                                    | i16 / 2,2                                         |
| BF16 vector2         | `<2 x i16>`                            | `<2 x i16>` / 4,4                                 |
| BF16 vector3         | `<3 x i16>`                            | `[3 x i16]` / 6,2                                 |
| BF16 vector4         | `<4 x i16>`                            | `[4 x i16]` / 8,2                                 |
| FP8 E4M3/E5M2 scalar | Distinct semantic formats, physical i8 | Selected record fields / 1,1                      |

BF3/BF4 whole-storage conversions extract/reconstruct raw lanes; BF2 keeps its required alignment.
Canonical equal-size vector bitcasts use scalar bitcasts and reconstruction, not synthetic i48 or
a second BF16 type. The [BF16/FP8 matrix](nvvm-backend-capability-ledger.md#bf16-and-fp8-role-specific-matrix)
owns exact role admissions, numerical conversion/overflow policies and exclusions. In particular,
integer-to-BF16 cannot silently round through Float32, FP8 literal overflow is not CUDA SATFINITE,
and runtime Float32-to-FP8 narrowing remains research-only.

Selected finite local records and their direct fixed arrays have narrower admission than general
recursive values. Natural AnyValue payloads are unpacked through canonical field keys into qualified
CUDA locals and repacked after mutation; earlier snapshots retain their bytes. Array value parameters
can use a separate callee Var without changing the caller snapshot. Internal out/inout array
references use the same Storage representation, including source array returns canonically rewritten
to OutParam. Calls and child-address plans require a local Var or a first-block mutable parameter of
an internal defined helper; type equality alone does not prove that origin. Native array/pointer
results, external/exported references, readonly borrows, wrapper/multidimensional forms and BF3/BF4
record-array combinations remain excluded even after a successful layout-cache lookup. Their exact
boundaries remain in the role matrix.

General per-root physical-storage rewriting is not implemented by these plans. Shared buffer-element
lowering already owns physical types and packing, but its selected roots do not include every Generic
local Ptr/Out/BorrowInOut form. Any reuse must preserve root selection and original semantic admission;
a global type replacement can erase the distinction between local, shared and external storage.

## Provider and vendor compilation

The optional provider owns isolated LLVM14.0.6 typed-pointer construction behind ABI46 opaque
handles. LLVM objects/symbols cannot cross into the CPU LLVM provider or host compiler. The separate
LLVM build is statically linked with hidden/excluded symbols. Handles belong to their live module;
destroying it invalidates subordinate handles. Serialization follows a caller-owned size-query/write
protocol, and the host copies outputs while retaining the provider library.

LLVM modules use `nvptx64-nvidia-cuda`, the specified DataLayout, `nvvmir.version` and kernel
annotations; a calling convention alone does not mark a kernel. The current direct emitter writes
verified **NVVM IR2.0 assembly** through the provider's audited compatibility writer. LLVM validation
and libNVVM verification are separate gates. Available assembly/bitcode serialization does not make
native bitcode the current direct-emission format. Vendor text-input deprecation leaves a qualified
production bitcode path as a readiness concern; compatibility follows actual dialect/toolkit evidence.

The downstream compiler accepts exact `Assembly + LLVMIR + Kernel` or `ObjectCode + LLVMIR + Kernel`
artifacts without byte sniffing. Each invocation creates a fresh `nvvmProgram`, adds the user module
and required coherent libdevice, verifies, compiles, retrieves diagnostics/PTX and destroys the program
on every path. Verification/compilation receive the same options. Failed diagnostics remain failed
artifacts; an empty vendor log falls back to its error string. Successful PTX drops the API trailing
NUL and rejects invalid payloads.

LibNVVM and libdevice must come from the same selected toolkit root. An explicit path wins;
discovery retains the actual loaded identity and queries NVVM/IR versions. A rootless loaded compiler
can work without libdevice, but a requested library requires a proven coherent root, with no fallback
to another toolkit's file.

For live named-library requirements, `INVVMCUDADeviceLibraryProvider` supplies a per-compilation
snapshot owning immutable bytes, path, origin reference and a private provenance token. It is parsed
once in an isolated input-library handle and every definition is queried before output creation.
Definitions require external linkage, default visibility, C convention, a fixed parameter list and
no return/parameter attributes. Failed loads return no handle; synchronous diagnostics are copied
immediately, and unsupported signatures return support=false without pretending to be parse errors.

The same compiler/token reaches downstream compilation through the appended options field. Provenance
is checked before vendor-program creation, and library addition consumes snapshot bytes without
reopening the path. Older option sizes default the field to null; legacy callers keep selected-path
loading. Dead helpers do not request a library, and LLVM-only calls need no libdevice. The older
timestamp-based cache hash is not an atomic filesystem snapshot. Named LLVM attributes are preserved;
library definition attributes are not copied onto output declarations.

The route passes explicit virtual architecture and O0/O3. Floating mode and Float32 denormal mode
are independent:

| Policy                           | libNVVM options                            |
| -------------------------------- | ------------------------------------------ |
| Default floating mode            | Vendor defaults for division, sqrt and FMA |
| Precise                          | `-prec-div=1 -prec-sqrt=1 -fma=0`          |
| Fast                             | `-prec-div=0 -prec-sqrt=0 -fma=1`          |
| Preserve/flush Float32 denormals | `-ftz=0` / `-ftz=1`                        |

Nondefault FP16/FP64 denormal policies and duplicate managed overrides reject before program creation.
NVRTC option aggregation differs; comparisons record effective options rather than equating labels.
The maintained three-mode corpus uses NVRTC O3 and NVVM O0/O3.

## Numerical and target-specific boundaries

CUDA output is comparison evidence, not a universal mathematical oracle. Core composition determines
evaluation order and intermediate precision; changing a named call or choosing a direct Half
instruction must preserve or explicitly revise that operation's contract. The current accepted
ordinary Half math paths generally widen to Float32 and narrow once. The exceptions `ceil`,
`floor`, `trunc` and `fma` select scalar Half LLVM intrinsics directly; the registry owns their
signatures. The provider lowers checked Half trunc to pure `cvt.rzi.f16.f16`, because the
qualified libNVVM 12.9 verifier rejects `llvm.trunc.f16`. Mechanical Half/i16 bitcasts satisfy its
inline-assembly operand rules; full module verification remains enabled.
Half FMA rounds the exact product-plus-sum once to nearest-even, without FTZ or
saturation, including in precise mode. This corrects Float32-intermediate double rounding.
Half-specific bit transport, BF16 operations and physical helper ABI remain separate mechanisms.
The [Half FMA contract](../../issue-nvvm-backend/RESULTS.md#native-half-fused-multiply-add-contract)
records independent finite bits and the limits of cross-backend comparisons. Approximate Half
exp2/tanh require separate accuracy-policy work; existing round ties-away remains unchanged.

| Maintained contract                                                                                                                                                                               | Limit that matters to implementation                                                                                                                                                                                                      |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [Round and directed rounding](../../issue-nvvm-backend/RESULTS.md#rounding-signature-and-numerical-contracts)                                                                                     | NVVM round retains ties-away; CUDA Half round uses ties-even. Directed functions preserve zero/Inf bits and compare NaN class.                                                                                                            |
| [Square root](../../issue-nvvm-backend/RESULTS.md#square-root-signature-and-numerical-contracts) and [fraction](../../issue-nvvm-backend/RESULTS.md#fraction-composition-and-numerical-contracts) | Selected intermediate evaluation and composition are part of the contract; bounded agreement is not universal equivalence.                                                                                                                |
| [Reciprocal square root](../../issue-nvvm-backend/RESULTS.md#reciprocal-square-root-numerical-contracts)                                                                                          | Independent references and explicit special/accuracy rules qualify the selected policies.                                                                                                                                                 |
| [Exp](../../issue-nvvm-backend/RESULTS.md#exponential-numerical-contracts) and [exp2](../../issue-nvvm-backend/RESULTS.md#base-two-exponential-numerical-contracts)                               | Test-defined spacing/encoding admission has explicit zero/maxfinite/infinity endpoints; empirical library tables are not guaranteed universal ULP bounds. CUDA Half FTZ/FMA/correction images differ from NVVM narrowing.                 |
| [Log family](../../issue-nvvm-backend/RESULTS.md#logarithm-family-numerical-contracts)                                                                                                            | CUDA Half ideal-RN checks are finite-corpus qualifications. CUDA double log10 narrows its input to Float32; preservation is not true-double accuracy. Negative tiny doubles narrowing to -0 are outside the common-classification corpus. |

The policies/checkers own exact constants, reference construction, correction images and negative
controls. Preserve the distinction between NaN-class accuracy and byte-for-byte baseline preservation,
including unpromised payloads. Do not fit tolerances to a new implementation's observed outputs or
overwrite historical failure outcomes when correcting a policy or implementation.

Several implementation exceptions carry independent contracts:

- **Narrow integers:** libNVVM/PTX can retain excess high bits in promoted16 arithmetic. Qualified
  signed/unsigned normalization at exact-width comparison/division/shift/widening/conversion consumers
  restores semantic low bits; physical Half/BF16 transport is not integer arithmetic. The retained
  signed16 O3 failure and correction remain in focused evidence.
- **Half conversion:** runtime Float32 narrowing uses `llvm.nvvm.f2h.rn` plus mechanical bitcast;
  ordinary fptrunc on the qualified stack could turn low-payload signaling NaNs into infinity.
  Shared literal narrowing is RN-even with discarded-bit information. NaNs promise class, not payload.
- **Waves and synchronization:** raw-bit all-equal distinguishes signed zeros and compares NaN payloads.
  Collectives execute before combining component predicates. Hardware activemask is effectful and
  distinct from logical participation; masked MIN/MAX preserves source order and finite Half seeds.
  CUDA match-all versus NVVM match-any remains a qualified distinction. Direct barrier/fence attributes
  do not establish transitive convergence metadata through arbitrary helpers. See [wave contracts](nvvm-backend-capability-ledger.md#waves-synchronization-and-observations).
- **Clocks:** selected side-effecting PTX prevents observed merging/hoisting of plain intrinsic reads.
  These are wrapping per-SM cycle counters, not global time or memory fences.
- **Nested stores:** a qualified libNVVM12.9 padding defect requires splitting at direct nested-struct
  boundaries with alignment derived from the parent guarantee. Arrays stay whole; terminal aggregates
  retaining struct-in-struct boundaries use conservative alignment1. The saved SSA value and layout
  stay unchanged. This does not prove scalability, and the NVRTC optimized integer-array copy failure
  remains open.

## Resources and evidence boundaries

Surface legalization runs before shared subscript expansion discards component masks. The canonical
collected field's static format selects physical payload types, explicit conversion and byte-X
coordinates; equal logical types can therefore access different formats. The provider emits typed
operations and mechanical bitcasts, not format discovery or implicit storage conversion. Component
updates preserve untouched raw lanes and remain non-atomic. In-range dynamic scalar components
select the converted replacement against each old physical lane; only the replacement is converted.
This reuses the canonical index value and adds no out-of-range or concurrent-write guarantee.
Arbitrary resource-helper provenance, additional packed/normalized formats and general aliases
remain outside the qualified boundary.

An undecorated `RWTexture<int4>` requires matching native32 four-channel storage. An opaque CUDA
handle does not make an RGBA8 allocation visible to the compiler, and shader reads using the same
wrong convention can conceal corrupt neighboring writes. The [physical surface contract](../../issue-nvvm-backend/RESULTS.md#physical-surface-correctness)
uses independent host readback and preserves existing NVRTC compile/rounding failures. Half stores
use RN-even, including subnormal/overflow boundaries; the NVRTC formatted-store truncation behavior
is separate. [The resource ledger](nvvm-backend-capability-ledger.md#texture-surface-and-descriptor-contracts)
owns exact shapes, native/Half format admissions and retained exclusions.

Texture operations use canonical sample/fetch/gather/query IR with typed resources, samplers,
coordinates and results. Existing ignored gather offsets and zero array-count outputs are not full
API repairs. Base geometry cannot determine allocated/view mip count. Opaque handles are not an
undocumented metadata interface; research driver-lookup failures remain evidence, even where ptxas
accepted the code. Texture/surface and length-one cube-array binding distinctions remain unresolved.

Host packing is part of each test/application contract. For example, CUDA column-major float3x2 uses
stride12/size24; graphics-packed inputs cannot silently be repacked to make a comparison pass.
NVRTC PCH ownership and subprocess cleanup must respect compiler/process lifetime. Preserve original
packing/process failures and the identity of any restored artifacts.

Compilation, physical readback, application runtime and performance qualify different claims:

- [Material runtime correctness](../../issue-nvvm-backend/RESULTS.md#material-runtime-correctness)
  uses explicit eval/sample entry layouts, two live synthetic textures and independent scalar oracles.
  Frozen oracle/PTX/profile identity, complete handle encoding, repeats/wraps and untouched sentinels
  remain required. Texel-center and linear-filtering profiles are distinct. Sampling's selected-layer
  throughput is not a full-material eval/PDF or distribution guarantee; independently frozen
  source-arithmetic candidates cannot be selected per component to fit output.
- [Original-input runtime](../../issue-nvvm-backend/RESULTS.md#original-input-corpus-dispatch-timing),
  [code quality](../../issue-nvvm-backend/RESULTS.md#original-input-corpus-code-quality) and
  [material measurement](../../issue-nvvm-backend/RESULTS.md#material-device-event-measurement)
  retain their own inputs, correctness prerequisites, timing scope and excluded claims. A compile
  pass or attractive PTX is not runtime correctness; faster compilation is not faster GPU execution.
- [Accepted baseline](../../issue-nvvm-backend/accepted-baseline.json),
  [accepted identity](../../issue-nvvm-backend/accepted-identity.json) and
  [focused evidence](../../issue-nvvm-backend/focused-evidence.json) retain actual tested identities,
  outcomes and resolved/unresolved failure transitions. New focused acceptance does not relabel an
  older full checkpoint as newly executed. STATUS and WORKFLOW determine the current economical
  validation scope; architecture does not impose a second campaign cadence.

Future changes should retire a complete duplicated ownership path with a demonstrated workload and
bounded validation. Generic physical-storage transformation, broader texture metadata semantics,
transitive convergence qualification and a production native-bitcode path remain separate work;
they are not implied by adjacent passing tests or by this document.
