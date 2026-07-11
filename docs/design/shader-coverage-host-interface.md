# Shader Coverage Host Interface

This document describes the host-facing contract for the hidden
synthetic resource that shader coverage injects, and how a host —
whether a direct Vulkan/Metal/D3D application, a CPU/CUDA-style
marshaling host, or `slang-rhi` itself — discovers, binds, and reads
back the coverage counter buffer.

For the user-facing workflow (CLI flags, LCOV conversion, examples)
see [`tools/shader-coverage/README.md`](../../tools/shader-coverage/README.md).
For the compiler-internal architecture see
[`shader-coverage.md`](shader-coverage.md).

## Problem and design split

Enabling any coverage tracing mode makes the compiler synthesize a
hidden bindable resource, the `__slang_coverage` counter buffer. That
buffer is deliberately invisible to Slang's public reflection — no
synthetic declaration leaks into `IComponentType::getLayout()`, IDE
views, or language-server output — yet a host must still be able to
discover it, bind real storage to it, and attribute the counter values
it reads back to source locations.

Two metadata channels split that job along its natural seam:

- `slang::ICoverageTracingMetadata` carries the **coverage
  semantics**: how many counters exist, what source location each
  coverage entry attributes to, and the typed input that
  `slang_writeCoverageManifestJson` serializes into the canonical
  coverage manifest.
- `slang::ISyntheticResourceMetadata` carries the **binding
  contract**: which hidden bindable resources exist, what kind they
  are, and where they live — as a descriptor-facing `(space, binding)`
  location or a CPU/CUDA-style marshaling location, depending on the
  target.

The split is intentional. `ICoverageTracingMetadata` stays
coverage-specific, while `ISyntheticResourceMetadata` is a generic
hidden-resource contract that future features (`printf` buffers,
profiling, sanitizers) can reuse without inventing a new discovery
mechanism. A host needs both halves: without the binding contract it
cannot allocate or bind the buffer, and without the attribution it
cannot interpret the numbers it reads back.

## The two metadata objects

Both objects are retrieved by calling `castAs(...)` on the
artifact-associated `slang::IMetadata` (from
`getEntryPointMetadata` / `getTargetMetadata`). They are owned by that
metadata container, immutable once returned, and safe for concurrent
read-only use while the owning COM object is alive. The normative
field-by-field reference is the doc comments in `include/slang.h`;
this section describes the semantics a host builds against.

**`ICoverageTracingMetadata`** answers "what do the counters mean".
Coverage entries are source-location based: the current producers emit
one entry per inserted marker op — line entries, function-entry
entries, and branch-arm entries. If generic specialization, inlining,
or other IR cloning creates multiple executable copies that map to the
same source location, those entries keep distinct counter slots; LCOV
export aggregates line entries by `(file, line)`, function entries by
function name, and branch entries by `(line, branch_site, branch_arm)`.
Hosts should always map entries to counters through
`CoverageEntryInfo::counterIndex` rather than assuming entry index
equals counter index — future source-region coverage may add ranged
entries with shared or derived counters without changing the binding
contract. (`getBufferInfo()` remains available as a legacy
coverage-specific binding query for ABI compatibility; new hosts
should bind through `ISyntheticResourceMetadata`, which also reports
the CPU/CUDA marshaling locations.)

**`ISyntheticResourceMetadata`** answers "where do I bind the buffer".
Coverage currently reports exactly one synthetic resource record — the
hidden `__slang_coverage` buffer, a global-scope read-write structured
buffer identified by a stable, opaque, non-zero `id` (treat the value
as opaque; do not hardcode it). Each record carries both location
styles, with sentinels marking whichever does not apply on the
compiled target:

| Field           | Sentinel | Meaning                                           |
| --------------- | -------- | ------------------------------------------------- |
| `binding`       | `-1`     | no descriptor binding is reported for this target |
| `space`         | `-1`     | the target has no descriptor-space dimension      |
| `uniformOffset` | `-1`     | no CPU/CUDA-style marshaling location is reported |
| `uniformStride` | `0`      | marshaling unavailable, or no stride applies      |

`0` is a valid value for `space`, `binding`, and `uniformOffset` —
test against the sentinels, not against zero.

## Reading the binding contract

There is one query surface, and it is deliberately low-level: Slang
reports the exact locations it chose, and the host routes them through
whatever descriptor-layout or parameter-marshaling machinery it
already owns. Slang does not define a second descriptor-layout
abstraction in `slang.h`.

```cpp
ComPtr<slang::IMetadata> metadata;
linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef());

auto* coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
    slang::ICoverageTracingMetadata::getTypeGuid());
auto* syntheticResources = (slang::ISyntheticResourceMetadata*)metadata->castAs(
    slang::ISyntheticResourceMetadata::getTypeGuid());

slang::SyntheticResourceInfo info;
if (syntheticResources->getResourceCount() == 1 &&
    SLANG_SUCCEEDED(syntheticResources->getResourceInfo(0, &info)))
{
    // Descriptor-backed targets: bind at (info.space, info.binding).
    // CPU/CUDA-style targets: patch the buffer view into the params
    // payload at info.uniformOffset.
    // Size the storage as coverage->getCounterCount() elements at the
    // width reported by CoverageBufferInfo::elementByteWidth.
}
```

After the dispatch, the host reads the counters back and uses
`ICoverageTracingMetadata` (or the serialized manifest) to attribute
them — the per-target recipes below spell out the binding step for
each backend family.

## Host-reserved spaces

`ISyntheticResourceMetadata` reports the final binding Slang chose. It
cannot, by itself, know about descriptor sets that exist only in the
host's runtime pipeline layout and are never referenced by the
compiled shader IR. Khronos descriptor-set hosts with such externally
owned sets should pass `-trace-coverage-reserved-space <space>` when
compiling (API: `CompilerOptionName::TraceCoverageReservedSpace`,
together with at least one coverage mode). The option is repeatable
and idempotent, and is an auto-allocation hint for whole descriptor
sets — an explicit `-trace-coverage-binding` still wins.
Auto-allocation treats each reserved space as occupied and reports the
resulting location through the metadata as usual.

The reservation policy applies to Khronos descriptor-set targets only.
Metal, CPU, CUDA, and D3D targets do not use this allocation model, so
the option is ignored with a warning there; D3D register-space
reservation is a follow-up design tracked at
[shader-slang/slang#11169](https://github.com/shader-slang/slang/issues/11169).

## Backend usage

| Backend / host style                    | Query path                                                                  | Binding action                                                                                                                     |
| --------------------------------------- | --------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| Vulkan / direct descriptor-backed hosts | `getResourceInfo(...)`                                                      | read `space` / `binding` and bind the coverage buffer using the host's descriptor-layout model                                     |
| Direct Metal hosts                      | `getResourceInfo(...)`                                                      | read `binding` as the `[[buffer(N)]]` index; `space == -1` is the expected sentinel (see "Direct Metal host binding recipe" below) |
| CUDA / CPU-style marshaling hosts       | `getResourceInfo(...)`                                                      | read `uniformOffset` / `uniformStride` from `SyntheticResourceInfo`                                                                |
| `slang-rhi` (planned)                   | `getResourceInfo(...)` while building `ShaderProgramSyntheticResourcesDesc` | `bindSyntheticResource(...)` once the companion slang-rhi support lands (see "`slang-rhi` consumption model" below)                |

D3D12 / HLSL hosts are expected to use the same `space` / `binding`
metadata shape; defining the D3D12 runtime binding policy is part of
[shader-slang/slang#11169](https://github.com/shader-slang/slang/issues/11169).

### Direct Vulkan host binding recipe

On Vulkan / SPIR-V targets the coverage buffer is an ordinary storage
buffer at the reported `(set, binding)`. Auto-allocation places it in
the descriptor set after the highest shader-visible or host-reserved
set, at binding 0, so it neither extends nor fills holes in a
user-owned set layout — which also means enabling coverage can add one
descriptor set to the pipeline layout. To bind it, a direct Vulkan
host:

1. reads `(space, binding)` from the coverage entry in
   `ISyntheticResourceMetadata` (or pins the location at compile time
   with `-trace-coverage-binding <binding> <set>`),
2. includes a descriptor-set layout for that set in the pipeline
   layout, with a `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` binding,
3. allocates a zero-initialized buffer of
   `getCounterCount() * elementByteWidth` bytes and writes it into a
   descriptor set at the reported location,
4. enables the device features the counter width requires (see
   "Counter element width and device requirements" below),
5. dispatches, then copies the buffer to host-visible memory and reads
   the counters back.

The executable references for this recipe are the two in-tree raw
Vulkan example programs,
[`examples/shader-coverage-image-pipeline`](../../examples/shader-coverage-image-pipeline/)
and
[`examples/shader-coverage-bvh-traversal`](../../examples/shader-coverage-bvh-traversal/),
which bind through exactly this contract and render LCOV reports from
the readback.

### Direct CPU host binding recipe

On the CPU target the synthesized `__slang_coverage` buffer is packed
into the same `GlobalParams` struct as user-declared global shader
parameters, and the generated kernel keeps the standard three-argument
ABI from `prelude/slang-cpp-types.h`:

```cpp
typedef void (*ComputeFunc)(
    ComputeVaryingInput* varyingInput,
    void* uniformEntryPointParams,
    void* uniformState); // <- the GlobalParams payload
```

A structured-buffer parameter is represented in that payload as a
`(data pointer, element count)` pair — the CPU prelude's
`RWStructuredBuffer<T>` layout:

```cpp
struct BufferView
{
    void* data;   // counter storage, counterCount elements
    size_t count; // counterCount
};
```

To bind the coverage buffer, a direct CPU host:

1. reads `uniformOffset` / `uniformStride` from the coverage entry in
   `ISyntheticResourceMetadata` (`uniformStride` equals
   `sizeof(BufferView)`; the synthesized field is appended after the
   user-declared globals, so `uniformOffset` never collides with them),
2. allocates `getCounterCount()` zero-initialized counter slots at the
   element width reported by `CoverageBufferInfo::elementByteWidth`,
3. writes a `BufferView` describing that storage into the
   `GlobalParams` payload at byte offset `uniformOffset`,
4. invokes the kernel and reads the counts back directly from the
   host-owned storage — no device readback step exists on CPU.

The executable reference for this recipe is the
`coverageCpuRuntimeDispatch` unit test
(`tools/slang-unit-test/unit-test-coverage-cpu-runtime.cpp`), which
binds through exactly this contract and validates exact per-line
execution counts for both counter widths.

### Direct Metal host binding recipe

Metal has no descriptor-space dimension: the synthesized
`__slang_coverage` buffer is an ordinary `[[buffer(N)]]` argument of the
kernel, where `N` is chosen by coverage auto-allocation to avoid the
buffer indices used by reflected shader parameters (or pinned with
`-trace-coverage-binding`). To bind it, a direct Metal host:

1. reads the coverage entry from `ISyntheticResourceMetadata` and uses
   `SyntheticResourceInfo::binding` as the buffer index; `space == -1`
   is the expected sentinel on Metal (no descriptor-space dimension),
   and the CPU/CUDA marshaling fields stay at their unavailable
   sentinels (`uniformOffset == -1`, `uniformStride == 0`),
2. allocates uint32 counter slots — MSL provides
   `atomic_fetch_add_explicit` only for 32-bit `atomic_uint`, so the
   compiler automatically caps counting-mode counters to 32-bit on
   Metal targets (an explicitly requested 64-bit width is capped with
   warning W45115); `CoverageBufferInfo::elementByteWidth` reports the
   effective width as usual,
3. allocates a zero-initialized `MTLBuffer` of
   `getCounterCount() * 4` bytes and sets it on the compute encoder at
   index `binding` (`setBuffer(counterBuffer, 0, binding)`),
4. dispatches, then reads the `uint32` counters back (directly from a
   shared-storage buffer, or via a blit for private storage).

The executable reference for this recipe is the
`coverageMetalRuntimeDispatch` unit test
(`tools/slang-unit-test/unit-test-coverage-metal-runtime.cpp`), which
compiles the emitted MSL with the Metal framework's runtime compiler,
binds through exactly this contract, and validates exact per-line
execution counts on a GPU.

### Direct CPU host binding recipe

On the CPU target the synthesized `__slang_coverage` buffer is packed
into the same `GlobalParams` struct as user-declared global shader
parameters, and the generated kernel keeps the standard three-argument
ABI from `prelude/slang-cpp-types.h`:

```cpp
typedef void (*ComputeFunc)(
    ComputeVaryingInput* varyingInput,
    void* uniformEntryPointParams,
    void* uniformState); // <- the GlobalParams payload
```

A structured-buffer parameter is represented in that payload as a
`(data pointer, element count)` pair — the CPU prelude's
`RWStructuredBuffer<T>` layout:

```cpp
struct BufferView
{
    void* data;   // counter storage, counterCount elements
    size_t count; // counterCount
};
```

To bind the coverage buffer, a direct CPU host:

1. reads `uniformOffset` / `uniformStride` from the coverage entry in
   `ISyntheticResourceMetadata` (`uniformStride` equals
   `sizeof(BufferView)`; the synthesized field is appended after the
   user-declared globals, so `uniformOffset` never collides with them),
2. allocates `getCounterCount()` zero-initialized counter slots at the
   element width reported by `CoverageBufferInfo::elementByteWidth`,
3. writes a `BufferView` describing that storage into the
   `GlobalParams` payload at byte offset `uniformOffset`,
4. invokes the kernel and reads the counts back directly from the
   host-owned storage — no device readback step exists on CPU.

The executable reference for this recipe is the
`coverageCpuRuntimeDispatch` unit test
(`tools/slang-unit-test/unit-test-coverage-cpu-runtime.cpp`), which
binds through exactly this contract and validates exact per-line
execution counts for both counter widths.

### CUDA hosts

CUDA uses the same uniform-marshaling contract as CPU: the buffer is
packed into the kernel's global-params payload, the metadata reports
`uniformOffset` / `uniformStride`, and the payload slot holds the same
`(data pointer, element count)` pair (the CUDA prelude's
`RWStructuredBuffer<T>` layout). The differences from the CPU recipe
are the ones inherent to the driver model: the pointer written at
`uniformOffset` must be a device pointer (e.g. from `cuMemAlloc`), the
params payload is passed to the kernel through the launch API, and
reading the counters back requires a device-to-host copy.

## Counter element width and device requirements

The synthesized `__slang_coverage` buffer defaults to `uint64` counters
(8-byte slots) and can be narrowed to `uint32` (4-byte) with
`-trace-coverage-counter-width 32`. The host must read the width from
`CoverageBufferInfo::elementByteWidth` (mirrored in the manifest as
`buffer.element_type` / `buffer.element_stride`) and allocate and read
back the buffer at the matching stride — do not assume 4 bytes.

A host driving compilation through the API selects the width with the
`CompilerOptionName::TraceCoverageCounterByteWidth` option. Note the
unit difference from the CLI flag: the API option accepts only `4` or
`8` (bytes), whereas the `-trace-coverage-counter-width` command-line
flag is a bit width (`32`/`64`). A value other than 4 or
8 — most easily produced by forwarding the bit width without dividing by
8 — fails codegen with `E45114 coverage-counter-width-bytes-invalid`
rather than silently selecting uint32.

The default 64-bit width requires runtime support for 64-bit integer
atomics, because the instrumented shader increments counters with a
64-bit atomic add. The host must enable the corresponding device
features before the coverage shader is created, or shader-module
creation is rejected and no counters are written:

- **Vulkan / SPIR-V** — enable `shaderInt64` (the SPIR-V `Int64`
  capability) and `shaderBufferInt64Atomics` (`VK_KHR_shader_atomic_int64`,
  core in Vulkan 1.2). The 64-bit path emits SPIR-V 1.5, so the instance
  must target Vulkan 1.2 (a 1.1 instance, max SPIR-V 1.3, rejects it).
  Query `shaderBufferInt64Atomics` via `VkPhysicalDeviceShaderAtomicInt64Features`
  in the `vkGetPhysicalDeviceFeatures2` pNext chain — this is the canonical
  struct for this feature and is the reliable path regardless of what the
  aggregated `VkPhysicalDeviceVulkan12Features` reports.
  Integrated GPUs frequently expose a compute queue but not
  `shaderBufferInt64Atomics`, so a host that enumerates devices should
  select one that advertises the feature (or fall back to
  `-trace-coverage-counter-width 32`).
- **HLSL / D3D12** — the `uint64` `InterlockedAdd` overload requires
  Shader Model 6.6 and the `Int64BufferAtomics` shader feature. Slang
  does not reject the 64-bit width when an older profile is requested;
  it emits the `uint64_t` `InterlockedAdd` call, and DXC then rejects
  the resulting HLSL at downstream compile. Callers targeting SM 5.x
  or SM 6.0–6.5 must pass `-trace-coverage-counter-width 32`.
- **Metal** — MSL provides no 64-bit atomic fetch-add at all; the
  compiler automatically caps counting-mode counters to 32-bit on
  Metal targets (an explicitly requested 64-bit width is capped with
  warning W45115), so no device opt-in exists or is needed.
- **CUDA / CPU** — no device opt-in; the backend selects the 64-bit
  atomic-add form directly.

`-trace-coverage-counter-width 32` removes these requirements and runs
anywhere 32-bit shader atomics work (notably MoltenVK on Apple Silicon,
which reports `shaderBufferInt64Atomics = false`), at the cost of silent
wraparound past 2^32 hits per counter slot.

## `slang-rhi` consumption model

The `slang-rhi` integration is planned work, tracked in
[shader-slang/slang-rhi#739](https://github.com/shader-slang/slang-rhi/pull/739);
the interface names below come from that in-flight change, while the
Slang side of the contract (the metadata objects above) is what it
consumes. The intended flow:

1. Slang compiles the shader and exposes `ICoverageTracingMetadata`
   and `ISyntheticResourceMetadata`.
2. The host converts the synthetic-resource metadata into a
   `ShaderProgramSyntheticResourcesDesc` passed through
   `ShaderProgramDesc.next`.
3. Backend layouts append the synthetic bindings into their normal
   internal layout model, and `slang-rhi` exposes the resolved
   locations through `ISyntheticShaderProgram`.
4. Runtime binding goes through `bindSyntheticResource(...)` or a
   direct `IShaderObject::setBinding(location.offset, ...)`.

The important design choice is that `slang-rhi` does not introduce a
separate backend-specific raw binding model in its core API: hidden
resources map into ordinary resolved `ShaderOffset`s, keeping the
runtime binding model uniform.
