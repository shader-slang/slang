# Transport resource-bearing aggregate values

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles and executes the existing
`tests/compute/dynamic-dispatch-bindless-texture.slang` and
`tests/compute/func-param-legalize.slang` fixtures. Their optimized IR contains ordinary structs
whose fields include CUDA texture and sampler values. Direct NVVM must transport those canonical
structs through structured-buffer loads, local storage, field extraction, and helper parameters
without reconstructing source declarations or adding a resource-struct-specific provider API.

## Progress

- [x] (2026-08-30) Completed Slice 126 as `b965b3578`; Release provider/host builds, both
  transcendental fixtures, CUDA PTX assembly, and the complete NVVM prefix passed 395/395.
- [x] (2026-08-30) Re-probed the remaining fixture census and captured the exact final IR for the
  five tests whose first diagnostic is `helper function parameter`.
- [x] (2026-08-30) Selected the two fixtures whose canonical producer is an ordinary resource-
  bearing value struct. Excluded `interface-func-param-in-struct.slang` because it independently
  requires a resource-bearing CUDA entry-point parameter ABI.
- [x] (2026-08-30) Defined one recursive, cycle-safe resource-aggregate classifier and used it at
  every selected
  producer/consumer boundary.
- [x] (2026-08-30) Added focused fake/real-provider coverage and promoted direct CUDA runtime/PTX
  lanes in both
  existing fixtures.
- [x] (2026-08-30) Inspected and assembled both PTX modules, passed Release provider/host builds and
  the complete 397/397 NVVM prefix, updated durable status, formatted, and self-reviewed.

## Surprises and Discoveries

- `dynamic-dispatch-bindless-texture.slang` is fully specialized before direct emission. Its
  helper is `Func(Float, MyImpl)`, where `MyImpl` has one `Texture2D` field, and its entry point
  obtains `MyImpl` with an ordinary `StructuredBuffer<MyImpl>` load.
- `func-param-legalize.slang` has `Func(float4, Param)`, where `Param` contains `Texture2D`,
  `SamplerState`, and `float`. The entry point initializes a local `Param`, loads it as one
  first-class value, and calls the helper; the helper uses keyed field extracts.
- The generic builder/provider already represents each resource leaf, LLVM structs, aggregate
  extraction, loads/stores, and generic calls. The gap is a set of compiler-side classifiers that
  still equate every ordinary value struct with the older numeric-only copyable family.
- A resource view can itself refer to an aggregate element type. A recursive classifier therefore
  needs an active-type set so a legal resource indirection does not make a recursive source struct
  recurse forever during preflight.
- After the shared aggregate contract passed preflight, the two fixtures exposed adjacent exact
  operations rather than another aggregate problem. `func-param-legalize.slang` samples a
  `Texture2D<float4>`, while `dynamic-dispatch-bindless-texture.slang` fetches a Float4 and calls
  scalar Float32 `trunc` before converting its selected lane to UInt32.
- The provider's texture intrinsic already returns an LLVM aggregate with four components. Typed
  Float2/Float4 sampling therefore needs only reconstruction of the requested LLVM vector from the
  first two or four fields; it does not need a new callback or a fixture-specific path.
- LLVM 14 can emit `llvm.trunc.f32`, but CUDA 12.9 libNVVM rejects that intrinsic as unsupported.
  The accepted spelling is the exact libdevice call `__nv_truncf`, which optimizes to PTX
  `cvt.rzi.f32.f32`.
- The first complete-prefix run exposed a validation regression in three existing Boolean-vector
  tests. Resource natural alignment is not the classifier for Boolean vectors, so the generalized
  availability check must preserve the established value-vector classifier alongside the new
  resource-capable alignment contract. The corrected prefix passes 397/397.

## Decision Log

- Decision: introduce one compiler-owned resource-aggregate contract rather than separate texture
  struct, sampler struct, buffer-element struct, and helper-parameter cases.
  Rationale: final IR has one semantic shape: a nonempty struct recursively composed of established
  first-class numeric values and established CUDA resource values. Every observed operation is an
  ordinary typed aggregate operation.
  Date/author: 2026-08-30, Codex.
- Decision: preserve the narrower copyable-aggregate classifier for byte-address payloads, helper
  results, and contracts that require numeric layout-copy semantics.
  Rationale: admitting a resource handle into a first-class helper parameter does not make it a
  byte-copy payload or a legal provider function result. The two roles remain meaningfully
  different.
  Date/author: 2026-08-30, Codex.
- Decision: defer `interface-func-param-in-struct.slang`.
  Rationale: its helper parameter shares this aggregate shape, but its entry point also receives
  that aggregate directly. CUDA launch-parameter representation and test-object binding are a
  separate ABI decision and should not be smuggled into this value-transport slice.
  Date/author: 2026-08-30, Codex.
- Decision: include the exact Float2/Float4 sampled-texture result and Float32 truncation operations
  exposed by the selected fixtures in this slice.
  Rationale: both are small, generic semantic rows on already-established descriptor interfaces,
  and stopping after aggregate preflight would leave neither selected existing fixture
  demonstrably complete. Texture vectors reuse the existing texture descriptor; truncation uses
  the existing typed value-operation descriptor and established libdevice-demand mechanism.
  Date/author: 2026-08-30, Codex.
- Decision: map Float32 truncation to `__nv_truncf`, not an LLVM intrinsic or text rewrite.
  Rationale: libNVVM rejected the otherwise valid LLVM 14 trunc intrinsic. The provider owns the
  exact physical mapping, libdevice demand is already catalog metadata, and the resulting PTX
  proves the operation remains native after libdevice optimization.
  Date/author: 2026-08-30, Codex.

## Context and Current Pipeline

After linking, specialization, CUDA varying legalization, and optimization, the relevant programs
are equivalent to:

    struct MyImpl { Texture2D tex; }
    float run(MyImpl value) { return value.tex.Load(int3(0)).x; }
    MyImpl value = source.Load(0);
    float result = run(value);

and:

    struct Param { Texture2D tex; SamplerState sampler; float base; }
    Param local;
    local.tex = diffuseMap;
    local.sampler = samplerState;
    local.base = -0.5;
    float4 result = run(local);

The provider lowers texture and sampler values to typed 64-bit handles and already accepts LLVM
structs in generic helper parameters. The direct emitter rejects the canonical Slang struct before
provider mutation because helper, local, field, structured-load, and alignment gates still use
`asNVVMSupportedCopyableStructType`, whose leaves are deliberately numeric-only.

## Scope and Non-Goals

In scope are nonempty resource-bearing value structs; recursive established numeric/resource
fields; cycle-safe classification; exact layout/alignment checks; resource-aggregate structured
buffer elements and loads; local struct storage and field addresses; first-class field extraction;
helper parameters/calls; focused fake coverage; both existing compute fixtures; direct runtime/PTX
lanes; typed Float2/Float4 sampled-texture results on the existing texture descriptor; exact scalar
Float32 truncation through the existing typed value-operation/libdevice contract; PTX assembly;
durable design status; and this plan.

Out of scope are resource-bearing helper results, arbitrary pointers in aggregates, parameter
groups, runtime-sized arrays as direct fields, recursive aggregate graphs, source interface or
witness operations, entry-point resource aggregates, compatibility aliases, new builder callbacks,
and unrelated fixtures whose next boundary is not this aggregate value contract.

## Architecture and Invariants

- One classifier owns the accepted recursive field set. Every consumer asks that classifier rather
  than duplicating a texture/sampler/buffer list.
- The existing copyable classifier remains the source of truth wherever values must be numeric and
  byte-copy/layout compatible.
- Resource views and handles use their established type resolvers. Aggregate admission cannot make
  an unsupported texture shape, sampler kind, buffer access, or element type legal.
- Recursive classification rejects an active type instead of silently accepting cyclic aggregate
  graphs.
- Aggregate field identity comes from the canonical struct key and exact field type. Emission uses
  the existing generic LLVM aggregate and call operations.
- Local and structured-buffer storage use the maximum natural alignment of the already-selected
  field representations and retain the existing CUDA-versus-LLVM layout check.

## Interfaces and Dependencies

Committed areas are direct NVVM type classification/lowering and validation/emission, the shared
semantic catalog, the LLVM provider, focused fake/real-provider coverage, the two existing compute
fixtures, durable design status, and this plan. Aggregate transport uses the existing generic type,
memory, extraction, and call operations. Forward-only builder ABI revision 24 adds only the exact
Float32 truncation operation ID; vector texture results fit the existing typed texture descriptor.

## Milestones

1. Add the recursive resource-aggregate classifier and natural-alignment query while preserving the
   narrower copyable contract.
2. Route structured-buffer element/load, local storage, keyed field access, helper signature/call,
   type lowering, reachable-type, and layout validation through the shared contract.
3. Add focused fake coverage proving the resource struct stays first class through local memory and
   a helper call, then promote both fixture lanes.
4. Inspect runtime/PTX results, assemble SM70 PTX, run Release/full gates, update docs and this log,
   format, perform the input-shape audit, and commit.

All four milestones are complete. The two adjacent semantic operations discovered after milestone
2 were completed through their existing generic descriptor families before fixture promotion.

## Validation and Acceptance

Acceptance requires focused fake coverage for the selected type topology and provider calls; all
existing plus new lanes of both promoted fixtures; PTX with expected helper calls or inlined
resource use and output stores; CUDA 12.9 `ptxas -arch=sm_70`; Release provider and host builds; the
complete `slang-unit-test-tool/nvvm` prefix; pinned formatting; and `git diff --check`.

The self-review inventories the classifier, recursion guard, alignment helper, every widened
consumer, and every retained special case. Remove any source-name match, duplicate resource-field
list, permissive structural equivalence, interface reconstruction, entry-parameter workaround,
provider fallback, or compatibility shim.

## Failure and Recovery

If either fixture exposes a distinct operation after the shared aggregate contract, retain the
optimized IR and diagnostics under ignored `build/slice127-*`, narrow this plan to the complete
shared subset, and record the next boundary. Do not weaken the fixture, inline helpers by source
name, flatten resource structs in the compiler, bypass exact layout/type checks, reset unrelated
work, or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, LLVM text, PTX, cubin, and logs under ignored `build/slice127-*`. Distill the
resource-aggregate value contract, exact CUDA evidence, and next measured corpus boundary into
`docs/design/nvvm-backend.md`, then commit this plan with the implementation as explicitly
requested.

## Outcomes and Retrospective

The resource-bearing structs remain canonical first-class LLVM aggregates across all selected
boundaries. One shared recursive classifier admits established numeric/resource leaves, rejects
recursive graphs with an active set, and supplies natural alignment. The emitter now applies that
contract to structured-buffer element/load validation, locals, keyed field addresses/extracts,
helper parameters/calls, reachable-type closure, and layout checking while preserving the narrower
copyable contract for byte payloads and helper results. No resource-struct provider callback,
source declaration reconstruction, compatibility alias, or fixture-specific path was added.

Focused fake coverage observes one resource struct loaded/stored through local memory and passed to
a helper whose keyed texture/sampler fields feed one typed Float4 sample. Real-provider coverage
serializes Float4 sampling in both LLVM dialects. ABI revision 24 adds exact Float32 truncation and
maps it to `__nv_truncf`; fake coverage proves libdevice demand and real coverage proves the exact
declaration/call in both serializations. The attempted `llvm.trunc.f32` mapping was removed after
libNVVM rejected it.

`dynamic-dispatch-bindless-texture.slang` passes 3/3 lanes. Its 919-byte direct PTX contains
`tex.level.2d.v4.f32.s32`, `cvt.rzi.f32.f32`, and the UInt32 output store; `ptxas -arch=sm_70`
emits a 2,792-byte cubin. The exact direct lane of `func-param-legalize.slang` passes and its
837-byte PTX contains `tex.level.2d.v4.f32.f32`, four Float32 additions, and four stores;
`ptxas` emits a 2,920-byte cubin. The whole latter fixture retains an unrelated pre-existing
Dawn/WebGPU bind-group validation failure. Release provider/compiler/unit-test builds pass, all
focused tests pass, and the complete NVVM prefix passes 397/397.

The input-shape audit retains the resource classifier/alignment query because the canonical final
IR intentionally contains resource values inside ordinary structs, and all consumers already use
generic aggregate operations. The cycle guard rejects a non-finite recursive representation at its
classification boundary. The widened consumers do not reconstruct syntax or recover interfaces.
The typed vector sample rebuilds the requested LLVM vector from the provider intrinsic's documented
four-field result, and exact truncation belongs in the provider because libdevice symbol selection
is a physical NVVM mapping. The only regression found during the revert/full-suite drill was the
accidental omission of established Boolean vectors from availability validation; retaining their
existing classifier alongside the new resource contract fixed the root classification error.
