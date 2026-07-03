# Shader Coverage Host Interface

This document describes the host-facing interface used for hidden
synthetic resources introduced by shader coverage, and how the same
contract is intended to serve:

- direct hosts that do not use `slang-rhi`
- `slang-rhi` itself
- descriptor-backed backends such as Vulkan, Metal, and D3D12
- uniform-marshaling backends such as CUDA and CPU

## Problem

Shader coverage injects a hidden bindable resource,
`__slang_coverage`. That resource must be:

- discoverable by hosts
- bindable by hosts
- attributable for reporting

The current design keeps coverage reflection-invisible and instead
uses two explicit metadata channels:

- `ICoverageTracingMetadata`
  - coverage semantics and attribution
- `ISyntheticResourceMetadata`
  - hidden bindable resource layout and binding data

## Design split

### 1. Coverage semantics

`ICoverageTracingMetadata` answers:

- how many counters exist
- what source file/line each counter maps to
- the typed input used by `slang_writeCoverageManifestJson` to
  serialize the canonical coverage manifest

This is the coverage-specific reporting layer.

### 2. Hidden binding semantics

`ISyntheticResourceMetadata` answers:

- what hidden bindable resources exist
- what kind they are
- what descriptor-facing binding they use
- what CPU/CUDA marshaling location they use

This is the generic hidden-resource binding layer.

That split is intentional:

- `ICoverageTracingMetadata` stays coverage-specific
- `ISyntheticResourceMetadata` can support future features such as
  `printf`, profiling buffers, or sanitizers

## Slang-side API

### Coverage metadata object

The coverage-specific public object is:

- `slang::ICoverageTracingMetadata`

Its intended use is:

- coverage reporting
- source-entry attribution
- coverage manifest serialization through `slang_writeCoverageManifestJson`

The coverage query methods are:

- `getCounterCount()`
- `getEntryInfo(index, ...)`
- `getBufferInfo(...)`
- `getEntryCount()`

These answer:

- how many runtime counters exist
- what each source coverage entry means
- the legacy coverage-specific descriptor binding view
- how many source coverage entries exist

Coverage entries are source-location based. The current producers emit
one source entry per inserted marker op: line entries, function-entry
entries, and branch-arm entries. If generic specialization, inlining,
or other IR cloning creates multiple executable copies that map to the
same source location, those entries keep distinct counter slots; LCOV
export aggregates line entries by `(file, line)`, function entries by
function name, and branch entries by `(line, branch_site, branch_arm)`.
Current entries all carry a runtime counter, but hosts should still use
`CoverageEntryInfo::counterIndex` instead of assuming entry index equals
counter index. Future source-region coverage can add ranged entries
that use direct counters, shared counters, derived counter expressions,
or no direct runtime counter of their own without changing the binding
contract.

Hosts use `ICoverageTracingMetadata` when they need to interpret the
counter values they read back, or emit LCOV or manifest output.
`getBufferInfo()` remains available for ABI compatibility with
coverage-specific descriptor-binding callers, but new host integration
should use `ISyntheticResourceMetadata` for binding because it reports
both descriptor bindings and CPU/CUDA marshaling locations.

### Synthetic resource metadata object

The hidden-binding public object is:

- `slang::ISyntheticResourceMetadata`

and its primary payload struct is:

- `slang::SyntheticResourceInfo`

Current key fields:

- `id`
  - stable synthetic resource identifier within the compiled program
- `bindingType`
  - Slang binding kind
- `arraySize`
- `scope`
  - `Global` or `EntryPoint`
- `access`
  - `Read`, `Write`, `ReadWrite`
- `entryPointIndex`
- descriptor-facing binding:
  - `space`
  - `binding`
- CPU/CUDA marshaling location:
  - `uniformOffset`
  - `uniformStride`
- `debugName`

Sentinels match `slang.h`: `binding == -1` means no descriptor
binding is reported, `space == -1` means the target has no descriptor
space dimension, `uniformOffset == -1` means no CPU/CUDA marshaling
location is reported for this target, and `0` is a valid value for all
reported offsets and bindings.

Coverage currently emits exactly one synthetic resource record for the
hidden `__slang_coverage` buffer. Hosts should treat its synthetic
resource id as an opaque, stable, non-zero identifier returned by the
metadata queries, rather than hardcoding a literal numeric value.

### Query functions

The core query methods are:

- `getResourceCount()`
- `getResourceInfo(index, ...)`
- `findResourceIndexByID(id, ...)`

This gives direct access to the complete resource record through
`SyntheticResourceInfo`. Descriptor-backed hosts read `space` and
`binding`; CPU/CUDA-style marshaling hosts read `uniformOffset` and
`uniformStride`.

### Raw binding queries

The raw binding API is the low-level query surface that exposes the
actual hidden binding locations without adding any descriptor-layout
policy on top.

For descriptor-backed paths:

- `getResourceInfo(index, ...)`
  - returns the explicit `(space, binding)` location and descriptor-
    facing binding properties for the synthetic resource

For CPU/CUDA-style marshaling paths:

- `getResourceInfo(index, ...)`
  - returns `uniformOffset` and `uniformStride`

For hosts that already own their runtime binding logic,
`getResourceInfo()` is the binding API. Slang does not add a second
descriptor-layout helper abstraction in this header; direct hosts map
the reported `bindingType`, `space`, and `binding` into their own
runtime descriptor model.

### Host-reserved spaces

`ISyntheticResourceMetadata` reports the final binding Slang chose. It
does not, by itself, know descriptor sets that only
exist in the host's runtime pipeline layout and are not referenced by
the compiled shader IR. Khronos descriptor-set hosts with such externally
owned sets should pass `-trace-coverage-reserved-space <space>` when
compiling, or set `CompilerOptionName::TraceCoverageReservedSpace`
through the API while also enabling at least one coverage mode, such as
`CompilerOptionName::TraceCoverage`,
`CompilerOptionName::TraceFunctionCoverage`, or
`CompilerOptionName::TraceBranchCoverage`.
The option is repeatable and duplicate values are idempotent. It is an
auto-allocation hint for whole Khronos descriptor sets; explicit
`-trace-coverage-binding` still wins. Metal, CPU, CUDA, and D3D targets
do not use this Khronos descriptor-set reservation policy in this PR, so
the option is ignored with a warning for those targets. D3D register-space
reservation is left to a follow-up design. Auto-allocation treats each
reserved space as occupied, then reports the resulting coverage binding
through `ISyntheticResourceMetadata`.

## Backend usage

| Backend / host style                                    | Query path                                                                  | Binding action                                                                                                                                 |
| ------------------------------------------------------- | --------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| Vulkan / Metal / direct descriptor-backed hosts         | `getResourceInfo(...)`                                                      | read `space` / `binding` and bind the coverage buffer using the host's descriptor-layout model                                                |
| CUDA / CPU-style marshaling hosts                       | `getResourceInfo(...)`                                                      | read `uniformOffset` / `uniformStride` from `SyntheticResourceInfo`                                                                            |
| `slang-rhi` Vulkan / CUDA backends                      | `getResourceInfo(...)` while building `ShaderProgramSyntheticResourcesDesc` | `bindSyntheticResource(...)` after `ISyntheticShaderProgram` resolves the location; provided by companion `slang-rhi` PR #739, not this PR     |

D3D12 / HLSL hosts are expected to use the same `space` / `binding`
metadata shape in a follow-up, but this PR does not define D3D register-space
auto-allocation or reservation policy.

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
- **CUDA / CPU** — no device opt-in; the backend selects the 64-bit
  atomic-add form directly.

`-trace-coverage-counter-width 32` removes these requirements and runs
anywhere 32-bit shader atomics work (notably MoltenVK on Apple Silicon,
which reports `shaderBufferInt64Atomics = false`), at the cost of silent
wraparound past 2^32 hits per counter slot.

## `slang-rhi` consumption model

The companion `slang-rhi` implementation is tracked in
`shader-slang/slang-rhi#739`.

The intended `slang-rhi` contract is:

1. Slang compiles the shader and exposes:
   - `ICoverageTracingMetadata`
   - `ISyntheticResourceMetadata`
2. The host converts synthetic-resource metadata into
   `ShaderProgramSyntheticResourcesDesc`
3. `slang-rhi` consumes that through `ShaderProgramDesc.next`
4. backend layouts append synthetic bindings into their normal
   internal layout model
5. `slang-rhi` exposes resolved binding locations through:
   - `ISyntheticShaderProgram` (provided by `slang-rhi` PR #739)
6. runtime binding uses:
   - `bindSyntheticResource(...)` (provided by `slang-rhi` PR #739)
   - or direct `IShaderObject::setBinding(location.offset, ...)`

The important design choice is that `slang-rhi` does not introduce a
separate backend-specific raw binding model in its core API. Instead,
it maps hidden resources into ordinary resolved `ShaderOffset`s.

That keeps the runtime binding model uniform.

## Binding Style

The current Slang interface exposes explicit binding metadata. It does
not define a second descriptor-layout abstraction in `slang.h`.

The host reads the hidden resource location and binds it directly:

- direct descriptor-backed hosts use `(space, binding)`
- direct CPU/CUDA-style hosts use `uniformOffset` and
  `uniformStride`
- `slang-rhi` hosts use the resolved location from
  `ISyntheticShaderProgram` (provided by `slang-rhi` PR #739) and then
  call:
  - `bindSyntheticResource(...)` (provided by `slang-rhi` PR #739)
  - or `IShaderObject::setBinding(location.offset, ...)`

This is the lowest-level path. It is appropriate for hosts that
already own their descriptor layout or parameter-marshaling logic and
just need an accurate hidden-resource contract from Slang.

The `slang-rhi` helper symbols in this section are companion-PR
interfaces tracked in `shader-slang/slang-rhi#739`; this Slang PR
defines only the metadata contract they consume.

## Direct-host model without `slang-rhi`

For direct hosts, the intended usage is:

1. compile via Slang C++ API with coverage enabled
2. get the linked or compiled artifact's `IMetadata`
3. call `castAs(...)` on that metadata to obtain:
   - `ICoverageTracingMetadata`
   - `ISyntheticResourceMetadata`
4. query `ICoverageTracingMetadata` for:
   - counter count
   - source entry count
   - source-entry attribution
5. query `ISyntheticResourceMetadata` for the hidden binding contract:
   - `getResourceCount()`
   - `getResourceInfo(...)`
   - read `space` / `binding` for descriptor-backed paths
   - read `uniformOffset` / `uniformStride` for CPU/CUDA-style
     marshaling paths
6. allocate and bind the hidden coverage buffer
7. dispatch
8. read counters back
9. use `ICoverageTracingMetadata` for reporting, LCOV, or manifest
    generation
