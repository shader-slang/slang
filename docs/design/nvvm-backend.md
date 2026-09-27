# Direct NVVM backend architecture

The experimental direct backend compiles linked Slang IR to PTX through libNVVM. It removes the
CUDA C++ emission step while sharing semantic checking and common lowering with the established
NVRTC route. NVRTC remains the default. Correctness, compile-time benefit and generated-code quality
are separate acceptance questions; selecting the direct route does not establish any of them.

This document describes current ownership and invariants. The [feature matrix](nvvm-backend-capability-ledger.md)
describes qualified combinations and exclusions. [STATUS](../../issue-nvvm-backend/STATUS.md) owns
current acceptance and loop authority; [RESULTS](../../issue-nvvm-backend/RESULTS.md) owns reproduction
commands. Historical experiments live in Git, rather than as additional architecture sections.

## Pipeline and source ownership

```text
Slang source → semantic checking → linked IR and shared lowering
    ├─ CUDA source legalization → CUDA C++ → NVRTC → PTX
    └─ NVVM legalization → preflight and emission plan → typed provider → libNVVM → PTX
```

The public target remains `SLANG_PTX`. Target-scoped `-emit-cuda-via-nvrtc` and
`-emit-cuda-via-nvvm` select one canonical option; the last explicit selector wins. The direct route
uses an internal NVVM artifact, not the CPU LLVM target. CUDA-family semantics, CUDA C++ preparation
and NVVM representation must have distinct owners when changing shared pipeline branches.

| Boundary                               | Owner and responsibility                                                                                                                                                            |
| -------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Linked IR and shared transformations   | [slang-emit.cpp](../../source/slang/slang-emit.cpp): `linkAndOptimizeIR`, specialization, shared semantic lowering and pass ordering                                                |
| NVVM-ready IR                          | [slang-ir-nvvm-legalize.cpp](../../source/slang/slang-ir-nvvm-legalize.cpp): `legalizeIRForNVVM`, typed intrinsic normalization, layout queries, selected bounds policy and cleanup |
| Type and representation classification | [slang-emit-nvvm-type-lowering.cpp](../../source/slang/slang-emit-nvvm-type-lowering.cpp): `NVVMTypeInfo`, use-specific provider types and caches                                   |
| Preflight and provider emission        | [slang-emit-nvvm.cpp](../../source/slang/slang-emit-nvvm.cpp): reachable functions, exact operations/signatures, addresses, layout proofs, diagnostics and emitted operations       |
| Immutable plan                         | [slang-emit-nvvm-plan.h](../../source/slang/slang-emit-nvvm-plan.h): owned recipes and checked instruction index                                                                    |
| Semantic operation authority           | [slang-nvvm-semantic-catalog.h](../../source/compiler-core/slang-nvvm-semantic-catalog.h): typed operation and overload contracts                                                   |
| Provider interface                     | [slang-nvvm-ir-builder-api.h](../../source/compiler-core/slang-nvvm-ir-builder-api.h) and builder facade: exact version negotiation and opaque handles                              |
| Physical LLVM construction             | [slang-llvm-nvvm.cpp](../../source/slang-llvm-nvvm/slang-llvm-nvvm.cpp): LLVM 14 typed pointers, NVVM metadata, generic instructions and qualified recipes                          |
| Vendor compiler lifecycle              | [slang-nvvm-compiler.cpp](../../source/compiler-core/slang-nvvm-compiler.cpp): coherent toolkit discovery, verification, compilation and diagnostics                                |

`legalizeIRForNVVM` runs after common linking and late bitcast normalization. It folds CUDA layout
queries, removes the canonical read-none `unmodified` check, discharges the selected CUDA derivative
requirement, applies the requested zero-index bounds policy, runs DCE and checks its postconditions.
Selected local Boolean-vector lane addresses are normalized to SSA lane updates before preflight;
this does not authorize escaping lane references.
`SLANG_ENABLE_BOUND_ZERO_INDEX` must become typed compare/select arithmetic because the direct route
does not preprocess the CUDA prelude. It preserves each access's own resource extent and index type.

Fixed standard-library producers carry typed NVVM intrinsic identities, which legalization consumes
as `IRNVVMIntrinsic`. The catalog does not infer these semantics from arbitrary CUDA source text.
Some richer texture, surface, atomic, scalar-out-parameter and compound-wave helpers still have exact
whole-body/signature recognizers. `RequirePrelude`, arbitrary GenericAsm and standalone execution
requirements are not general no-ops; their meaning must be owned before they can be removed.

A shared producer fix belongs before this boundary when its IR shape or semantics are wrong. For
example, aggregate receiver snapshots must be established before deferred buffer loading. Flattening
aggregate parameters before `deferBufferLoad` preserves the original value across subsequent resource
writes; emission must not reread storage to reconstruct a saved semantic value.

## Preflight is a contract, not a trial emission

Preflight validates reachable functions, entry and helper signatures, typed operations, operand
relationships, layout and target requirements before provider module/program mutation. Unsupported
forms return a source diagnostic. Unit tests assert both rejection and zero provider/module/program
creation for important negative boundaries.

`NVVMEmissionPlan` owns reachable function order, collision-checked physical names and source-keyed
records for ordinary value operations and several compound scalar, memory and resource families.
`NVVMEmissionPlanIndex` enforces unique sources and supplies typed lookups to emission. Provider
requirements are deduplicated by exact overload; emission records remain one per canonical source
instruction. Requirements are checked before constructing a module.

Ordinary local allocations, loads and stores have required source-keyed plan records. Allocation
planning owns the admitted type use and physical alignment. After pointer/value/dominance validation,
load/store planning owns alignment, load flags, storage conversion and pointer-value ABI/provenance
choices. Emission consumes these records without repeating those decisions. BF2 uses an identity
recipe; BF3/BF4 recipes carry the canonical vector, lane count and conversion result use. Compact
parameter-group vector recipes remain distinct from readonly native-vector access. A Generic Read
borrow may refer to mutable caller storage and therefore does not imply invariant-load metadata;
that requires the separate immutable-location contract.

Field and indexed-element addresses also have required source-keyed recipes. `NVVMAddressPlan`
records canonical field selection, admitted storage/access roles and the raw-offset or sequential
pointer operation. Its source-to-index dictionaries serve both preflight and immutable emission,
without scanning unrelated module addresses. Pointer validation and ordinary memory planning reuse
these records; selected address emission does not rerun the recognizers. Canonical IR pointer types
remain authoritative for pointee, access qualifier and address space.

Preflight preserves its two-stage diagnostic order. Direct device/shared element-pointer results
retain pending recipes until their original second-stage raw/sequential/device-array recognition;
all recipes must be complete before emission. Ordinary load/store planning computes its canonical root after pointer validation. Read-only selection permits reads but does not itself authorize invariant-load metadata.

The plan boundary is incomplete: initial ancestor admission remains recursive, and some scalar
intrinsic and wave families still resolve during both validation and emission. Field-value
extraction, dedicated resource operations, helper-signature classification and general
structured-storage conversion retain repeated or recursive decisions. This is current
architectural debt, not a reason to bypass preflight or add a second semantic catalog.

## Canonical types and use-specific representation

Canonical Slang types, field keys and instructions remain the semantic source of truth. Physical
LLVM equality does not imply semantic equality: Half and BF16 both occupy 16 bits; the two FP8
formats both occupy 8 bits. Their conversions and operation admission remain distinct.

`NVVMTypeLoweringContext` caches an `NVVMTypeInfo` for each canonical linked-IR type. Its nine uses are:
entry result, helper result, entry parameter, helper parameter, helper value, ordinary value, local
storage, parameter-group storage and structured-buffer storage. `supports` owns admission for
provider type lowering. Other helper/address gates still contain overlapping role proofs.

Role validation happens before a cache lookup. Value, helper and the different storage representations
have separate caches; a helper pointer key includes its pointee use. A successful storage lookup must
never authorize a previously unsupported value, resource, exported signature or reference role.

The recursive copyable/helper domains support selected finite arrays and records. Specialized local
substandard-float records have a narrower proof. Broadening a general recursive predicate to admit
one local type can also admit device, shared or external storage: changes must name the intended role.

### Address provenance and layout

Address-space 0 is generic/code, 1 global, 3 shared, 4 constant and 5 local. Address-space conversion
uses `addrspacecast`, preserving pointer provenance. Integer round trips cannot replace it.
Immutable loads from constant-memory parameter-group fields remain ordinary loads; read-only global
load recipes apply only to their qualified device-buffer pointers.

Canonical pointer shapes carry access, address space and sometimes physical-layout operands.
A field address is resolved by its canonical key and qualified parent, not by pointee type alone.
Array element addressing must retain the same proof through its index and element type. A physically
representable leaf does not establish permission for every pointer that names it.

The exact one-operand Generic `Ptr`, `OutParam` and `BorrowInOutParam` roots qualify selected local
mutable substandard storage. Readonly references, exported signatures and resource/shared/device
storage have independent boundaries. Public pointer results often become `UserPointer`; they do not
exercise the same shape as synthetic Generic pointer results.

Natural, CUDA and physical LLVM layout are separate contracts. AnyValue uses Natural payload packing.
Internal copyable locals and helper borrows may use native LLVM value layout; for example, native
float3 storage has alignment 16. External CUDA storage boundaries and explicitly qualified local
BF16/physical-storage families use their selected CUDA-compatible representations. Internal helper
storage is not a general CUDA ABI. `getSizeAndAlignment` caches layout rules separately. Qualified
aggregate-storage layout checks verify offsets, size and alignment for the selected role before
allocation. Increasing allocation alignment cannot repair wrong member offsets or array stride in
a physical type.

Storage conversion is explicit where representations differ: Boolean storage, compact numeric
vectors, physical matrices and BF16 vectors cannot inherit register layout without proof. Ordinary BF16 and compact-vector memory conversions execute checked plan recipes. General recursive
structured-storage conversions still happen in the emitter. Shared
[buffer-element lowering](../../source/slang/slang-ir-lower-buffer-element-type.cpp) already provides
physical types and packing/unpacking operations. Its discovery currently selects resources and
UserPointer/Input/Output roots, not Generic local Ptr/Out/BorrowInOut roots. Reuse for local storage
needs explicit root selection and preserved semantic-role admission; globally replacing Generic
pointer types with ordinary arrays could erase the exclusions that preflight must enforce.

### BF16 and FP8 contracts

| Semantic value   | Register/internal value                  | Qualified local storage     | CUDA size/alignment |
| ---------------- | ---------------------------------------- | --------------------------- | ------------------- |
| BF16 scalar      | `i16`                                    | `i16`                       | 2 / 2               |
| BF16 vector2     | `<2 x i16>`                              | `<2 x i16>`                 | 4 / 4               |
| BF16 vector3     | `<3 x i16>`                              | `[3 x i16]`                 | 6 / 2               |
| BF16 vector4     | `<4 x i16>`                              | `[4 x i16]`                 | 8 / 2               |
| E4M3/E5M2 scalar | distinct semantic formats, physical `i8` | selected record fields only | 1 / 1               |

BF3/BF4 CUDA forms are component structs. LLVM value-vector alignment would change containing
record layout; using a scalar array for BF2 would instead lose its required alignment. Whole local
BF-vector loads/stores convert by extracting and constructing raw lanes, without numerical conversion.
The internal BF-vector helper ABI is not an external CUDA helper ABI. Explicit vector Select,
component-pointer access and aggregate membership need their own proof; branch/phi transport alone
does not qualify all three.

Canonical equal-size vector bitcasts are lowered by the existing bitcast producer into scalar
bitcasts, extraction and reconstruction. No synthetic source i48 type or second BF16 semantic type
is needed. Both AST and IR CUDA layout queries retain canonical BF16 element identity; Half3/Half4,
ushort vectors and Natural layout keep their distinct rules.

BF16↔Float32 uses the qualified SM80 conversion recipe. Narrowing is nearest-even
`cvt.rn.bf16.f32`; widening shifts the payload into Float32's high word. Raw transport and widening
preserve payload bits on the qualified target; narrowing NaNs promise classification only. Canonical
literal recovery uses the shared producer's bits, including signed encodings at the integer builder
boundary. This does not license integer→BF16 through Float32: that can double round. For example,
integer 16842753 should narrow to BF16 bits `0x4b81`, while nearest Float32 then BF16 gives `0x4b80`.

BF16 dot for widths 2–4 starts at positive zero and rounds each product and sum separately in source
lane order. Two BF16 FMA instructions per lane implement those separately rounded operations on SM80.
Unrestricted Float32 accumulation or a fused product-plus-accumulator changes the contract. General
BF16 arithmetic, comparison, integer and Half/double conversions remain separate admissions.

FP8 transport preserves all bytes, including nonfinite encodings. Same-width signed/unsigned and
cross-format bitcasts preserve bits; same-format Select preserves semantic type. Runtime widening to
Float32 is exact for finite values and E5M2 infinities; NaN sign/payload are unspecified. E4M3 maximum
is 448, minimum subnormal 2^-9, with magnitude byte 127 NaN; E5M2 maximum finite is 57344, minimum
subnormal 2^-16, with exponent 31 reserved for infinities/NaNs. The provider uses integer decoding and
exact powers of two, without native FP8 instructions on SM80.

Shared finite FP8 folding uses nearest-even subnormal rounding and exact widening. Its overflow
policy is intentionally not CUDA constructor SATFINITE: E4M3 values above 448 become signed NaN;
E5M2 retains the rounded overflow boundary 61440 and infinity behavior. Nonfinite FP8 literals are
rejected before provider discovery. Runtime Float32→FP8 narrowing is research-only; the backend must
not hide this producer-policy distinction by reconstructing literals.

### Local records and AnyValue

Whole internal record values admit finite nonempty record trees whose leaves are integer scalars,
FP8 scalars, BF16 scalar or BF2, with at least one substandard descendant. Integer-only child records
may accompany them. Value and local storage agree for these leaves. A separate local-only flat BF16
record family also permits BF3/BF4 fields, with component-array storage; it does not grant whole-record
value or nested BF3/BF4 membership.

For example, `{uint16_t before; BF2 value; uint16_t after;}` has Natural offsets 0/2/6 and size 8,
alignment 2, but CUDA offsets 0/4/8 and size 12, alignment 4. AnyValue unpacking uses canonical field
keys to create a CUDA local record from Natural payload bytes. A mutating interface wrapper unpacks,
invokes the concrete method, reloads and repacks the result. An earlier interface copy retains its
saved payload. Runtime-selected nested regressions explicitly qualify both concrete conformers and
those mutation/snapshot paths.

Record arrays remain excluded from this substandard domain, including local indexing and retained
mutable helper copies. BF3/BF4's physical lane arrays and successful layout metadata queries do not
establish runtime record-array support. Generic local-record pointer helper results remain excluded;
the exact synthetic Generic result boundary has source-review evidence rather than canonical-source
execution coverage.

## Provider and downstream compiler

The optional `slang-llvm-nvvm` provider owns an isolated LLVM 14.0.6 typed-pointer construction path.
It exports a versioned Slang C ABI with opaque handles and one generic operation surface. Current
provider ABI is 42; compiler and provider must negotiate the exact required interface/capabilities.
Raw LLVM objects and symbols must not cross into the CPU LLVM provider or the host compiler.
Handles belong to their creating live module; destroying it invalidates subordinate handles. ABI
buffers remain caller-owned, and serialization uses a size-query/write protocol. The host retains
the provider library, validates returned handles and copies output into its own blob.
The separate LLVM build is statically linked with hidden/excluded LLVM symbols, avoiding a competing
process-visible dynamic `libLLVM` dependency.

NVVM uses 64-bit `nvptx64-nvidia-cuda`, the specified NVVM DataLayout, explicit `nvvmir.version` and
kernel annotations. A calling convention alone does not mark a kernel. A valid LLVM module may still
be invalid NVVM: libNVVM verification remains mandatory. The current direct emitter explicitly serializes verified NVVM IR 2.0 assembly through the
provider's audited compatibility writer. LLVM verification runs before serialization and vendor
verification remains a separate gate. The provider also exposes assembly/bitcode serialization, but
that does not make native bitcode the current direct-emission format. Text input is deprecated by
the vendor, so a qualified production bitcode path remains a readiness concern. Compatibility is
constrained by actual toolkit/dialect evidence, not the CPU LLVM version or a toolkit folder name.

The downstream compiler accepts exact `Assembly + LLVMIR + Kernel` or
`ObjectCode + LLVMIR + Kernel` artifacts. It does not infer the format by sniffing bytes. Each compile
creates a fresh `nvvmProgram`, adds the user module, optionally adds coherent libdevice, verifies,
compiles, retrieves logs/PTX and destroys the program on all paths. Verification and compilation use
the same options. Failed vendor diagnostics stay failed artifacts; an empty vendor log falls back to
its error string. The API trailing NUL is removed from successful PTX and invalid payloads are rejected.

libNVVM and libdevice must come from the same selected toolkit root. An explicit NVVM path wins;
logical loader discovery and deterministic filesystem candidates retain actual loaded identity.
`nvvmVersion` and `nvvmIRVersion` are queried; `nvvmLLVMVersion` is optional and target-dependent.
A rootless loaded compiler may compile without libdevice, but requested libdevice requires a proven
coherent root. There is no fallback to another toolkit's file. The selected library and coherent
libdevice identities belong in provenance and cache decisions.

The direct route passes an explicit virtual architecture and optimization 0 or 3. Floating policy
and Float32 denormal policy are independent:

| Policy                             | libNVVM options                                        |
| ---------------------------------- | ------------------------------------------------------ |
| Default floating mode              | leave precise division/sqrt and FMA at vendor defaults |
| Precise                            | `-prec-div=1 -prec-sqrt=1 -fma=0`                      |
| Fast                               | `-prec-div=0 -prec-sqrt=0 -fma=1`                      |
| Preserve / flush Float32 denormals | `-ftz=0` / `-ftz=1`                                    |

Nondefault FP16/FP64 denormal policy and duplicate overrides of managed options are rejected before
program creation. NVRTC's option aggregation differs; differential experiments record effective
options instead of assuming that similarly named optimization levels imply identical math policy.
NVRTC comparison uses O3 in the maintained three-mode corpus; NVVM runs O0 and O3.

## Target-specific exceptions that must remain explicit

**Nested stores.** Installed libNVVM 12.9 can combine narrow fields across nested-record padding,
also for integer-only records. Provider `_emitStore` validates the original operation, then splits
at direct nested-struct boundaries using canonical LLVM field types and DataLayout. Child alignment
comes from the actual parent guarantee via `commonAlignment`.

Arrays stay whole. `_containsNestedStructLayout` follows canonical array/struct types and stops at
pointers; a terminal whole store containing a remaining struct-in-struct boundary gets conservative
alignment 1. Allocation/load alignment, signatures and the saved SSA value remain unchanged. This
avoids element unrolling and source rereads. Flat-record arrays retain their alignment. Tests include
root/wrapped/multidimensional integer arrays and 39 serialized shape/alignment combinations. Large
65,536-element O3 experiments hit the same 120-second/4-GiB bound with and without the annotation;
there is no scalability or speed claim. NVRTC's optimized integer-array copy defect remains open.

**Clocks and masks.** Clock reads use side-effecting inline PTX because the tested intrinsic path
merged/hoisted live observations. They are per-SM wrapping cycle counters, not a global wall clock or
memory fence. Hardware `activemask` is distinct from logical participation synthesis. CUDA quad
helpers preserve complete-source-quad/matching-shuffle semantics; partial quads have no defined oracle.
Masked floating MIN/MAX retains source comparison order and identities, including Half's finite
exclusive seeds ±65504 and payload-preserving ordered selection. Algebraic reassociation is unsafe.

**Texture queries.** Selected non-mip geometry has direct lowering. Full mip, array-count and
allocated/view-level-count semantics remain unresolved. CUDA source helpers ignore requested mip and
write zero for some counts. A length-one declared cube array may bind a nonlayered cube. Texture and
surface handles refer to different resources/mip contracts; opaque handle internals are not metadata.
`txq.level.width` executed in research, while array-size/level-count query cubins failed driver lookup
on the qualified stack despite assembling. Geometry cannot reconstruct allocated level count.
Research observed layer count in 1D-array height and 2D-array depth, and cube count in cubemap-array
depth (host array depth counts faces). These observations do not resolve length-one binding or every
subresource view. Full and partial mip chains can have identical base dimensions.

**Host packing and process lifetime.** The column-major `float3x2` CUDA layout is compact stride 12,
size 24; graphics-packed host words do not implement that contract. The permanent
[compact-column fixture](../../tests/cuda/nvvm-column-major-compact.slang) checks all six elements
and multiplication in three modes, with fresh 24-byte reflection. Raw test-buffer uploads preserve
their authored bytes; do not silently repack them for a different target. Keep the original mismatches.
NVRTC automatic PCH directories are private to each compiler owner; shared persistent paths can
outlive compatible state. Owned-process cleanup must account for surviving descendants after a leader
exits, and temporary observers must restore compiler/module/configuration bytes before acceptance.

## Remaining refactoring direction

Ordinary allocation/load/store planning and checked field/index recipes establish an analysis and
backend-recipe boundary. They do not rewrite physical storage into Slang IR or consolidate every
address proof. Canonical IR
types remain the source of semantic identity, while planned type uses and conversions describe their
physical roles. A successful physical-type lookup still cannot authorize another role.

A future transforming pass should reuse existing physical-storage lowering after defining per-root
selection/specialization and retained semantic admission. Field/index recipes already supply checked
facts to pointer validation, ordinary memory planning and emission. A broader address analysis could
consolidate their remaining recursive ancestor admission and extend that authority to other address
producers and consumers. It must not invent a second type hierarchy. Remaining compound recipes can move
into the existing immutable plan as their families are touched. File separation should follow these
ownership boundaries.

A transforming local-storage pass is deferred. A one-record rewrite would leave the existing
BF3/BF4 conversion path necessary while adding explicit root selection, callee specialization,
original semantic admission and generated-helper cleanup. The shared lowerer's current type-wide
entry and fallback for unrecognized pointer uses do not establish safe per-root isolation. Revisit
this when a real workload motivates retiring a complete representation family; an isolated passing
rewrite alone would not demonstrate that architectural benefit.

These remaining proposals are not implemented support or authorization to resume general feature
work. Record-array admission would be a useful later test of the revised boundary.

Material runtime qualification has a separate contract from compile/assembly and static resource
measurements. The maintained tiled-brass validator runs the unchanged `eval_buffer` and
`sample_buffer` with two live synthetic textures and independent scalar references in all three
modes. One CUDA driver owns textures, buffers and cleanup; an explicit entry contract selects
packing, inputs and output checks. Installed-header assertions and fresh PTX review establish the
168-byte global block and input/output strides (40/16 for eval, 24/32 for sample). Execution requires
byte-identical reviewed PTX and frozen oracle hashes. Full texture handles must fit the application's
low30 encoding without truncation. Repeats, wrapped UVs and untouched tail sentinels are checked.

Sampling returns selected-layer throughput, not the collapsed full-material eval/PDF estimator.
Cancellation in the source's artistic-IOR calculation affects average Fresnel and selection weights.
The sample oracle therefore has two independently derived source-arithmetic candidates, fixed before
GPU execution. One candidate must explain every record in a mode within the unchanged residual
budget; components cannot choose candidates independently. This finite qualification does not prove
a unique generated instruction sequence or cover every legal optimization. Exact flags and an
early-rejection zero record complement the numerical checks. The shared driver preserves eval's
original inputs, oracle and tolerance. Original assets, live LUT reads, arbitrary graph/input
composition, sampling-distribution accuracy and application performance remain separate qualifications.
See [the validator](../../extras/validate-nvvm-material-runtime.py) and
[commands](../../issue-nvvm-backend/RESULTS.md#material-runtime-correctness).

The synthetic device-event runner reuses that driver and oracle at 65,537 and 1,048,577 records.
Each input is the original record at `index % 65`; the shader uses dispatch identity only for bounds
and indexing, so every output must repeat its independently verified tile exactly. Six small and
twelve enlarged correctness cells precede timing. A full qualified reference is bound to entry,
count, backend, cubin, input and oracle hashes; measurement rechecks those bindings and correctness.
Every warmup and measured launch resets active output and guards, then checks all downloaded bytes
against that reference. Failed launches, cleanup, comparisons or observed competing processes cannot
produce accepted timing summaries.

CUDA events bracket one kernel on the same stream. Compilation, allocation, reset and readback are
excluded; host submission gaps can still contribute to the interval. Three warmups and nine samples
per cell run in two rounds with reversed complete order. All samples remain available; intervals
below the predeclared 0.1 ms threshold have no throughput summary. Tiny hot textures, periodic inputs,
known sample rejections and correctness transfers between launches limit the result to this
synthetic protocol. Clock observations are recorded without changing device configuration. See the
[measurement runner](../../extras/measure-nvvm-material-runtime.py) and
[protocol](../../issue-nvvm-backend/RESULTS.md#material-device-event-measurement).

The measured eval gap does not currently motivate another NVVM lowering pass. Both optimized PTX
paths already contain no helper calls, and NVVM O3 reports no local stack. A scoped diagnostic that
removes only NVRTC stores to proven never-read local byte ranges lets `ptxas` eliminate that path's
local stack and removes most of its measured gap. The correct receiver snapshots remain part of
language value semantics. A source-derived CUDA reduction isolates bounded dynamic indexing:
eight constant-index variants eliminate local storage, while eight bounded-index variants retain
27 stores to proven never-read bytes alongside live stack data. Initialization spelling, receiver
snapshot size and branch spelling produce identical PTX within each group. Three full-material
force-inline controls produce the original PTX unchanged. These are static optimization findings,
not reduced-kernel runtime or performance evidence, and do not identify a particular vendor pass.
Keep a source-level reproducer and general field-liveness proof; do not turn the diagnostic's
exact-PTX deletion rule into an emitter workaround. The measured control covers deletion plus
downstream reoptimization without identifying a unique hardware bottleneck. Current measurements
and proof provenance remain in
[focused evidence](../../issue-nvvm-backend/focused-evidence.json).
