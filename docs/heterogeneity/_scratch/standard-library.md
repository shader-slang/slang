# Bundled standard modules

`h-slang` ships runtime and compute helpers with the compiler. Import the APIs you need;
their declarations are not added to every program. No additional checkout or `-I` option is required.

| Import                            | Provides                                                                                                       |
| --------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| `slang.runtime`                   | Core helpers, file/image I/O, resource operations, grid dispatch, rasterization, ray tracing, and window/input |
| `slang.tensor`                    | Descriptor-backed tensors of rank 1–4, fill/copy/map, ordinary reductions and inner products                   |
| `slang.tensor.differentiable`     | The tensor API plus gradient allocation, `dmap`, `dsum`, and value/readback/indexing derivatives               |
| `slang.compute.buffer_utils`      | Buffer initialization, mapping, and reduction                                                                  |
| `slang.compute.texture_utils`     | Texture initialization, mapping, and reduction                                                                 |
| `slang.resource.resizable_buffer` | Scratch-buffer allocation and descriptor snapshots                                                             |

Runtime implementation modules remain available individually, for example
`slang.runtime.compute.grid`, `slang.runtime.io.byte_stream`, and
`slang.runtime.window.simple_input`. Buffer and texture operations each live in a single
utility module, so initialization, mapping, and reduction share one import.

`slang.runtime` also exports `Range`, `range`, and `BufferView` from
`slang.runtime.resource.buffer_view`. See
[resource indexing and transfers](resources.md#resource-indexing-and-transfers).

```slang
import slang.runtime;
import slang.compute.buffer_utils;
import slang.compute.texture_utils;

[host]
void main()
{
    let values = RWStructuredBuffer<float>::alloc(73);
    values.fill(2.0f);
    values.mapInPlace((float value) => value * 3.0f);
    let total = RWStructuredBuffer<float>::alloc(1);
    values.reduceTo(total, (float a, float b) => a + b);
    print(total[0]); // 438.000000

    let image = RWTexture2D<float>::alloc(19, 9);
    image.fill(1.0f);
    image.mapInPlace((float value) => value + 1.0f);
    image.reduceTo(total, (float a, float b) => a + b);
    print(total[0]); // 342.000000
}
```

Host operations require the host-graph runtime; shipping a module
does not make every operation available on every target. In particular, native windows and
ray tracing are not browser APIs. `slangi` enables experimental features automatically;
direct compiler clients using tensors need them for `slang.numerics` and custom derivatives.

Buffer and texture reductions require an associative combine function. A zero-element
buffer reduction leaves its destination unchanged; the tensor reduction API instead accepts
an explicit identity for empty inputs. See [resources](resources.md) for tensor storage
contracts and [automatic differentiation](automatic-differentiation.md) for gradient lifetimes.

The browser slangvm package ships the corresponding sources under its package search root.
Native installations ship compiled modules under `slang-standard-module-*` beside the Slang
library. Keep that directory with the tools and rebuild it whenever the compiler changes.

## Rough edges

- These APIs retain their experimental host-graph and autodiff requirements. Direct
  `slangc` use needs `-experimental-feature`; embedding clients use
  `CompilerOptionName::ExperimentalFeature`. `slangi` sets this option for you.
- Compiled modules must match the compiler that produced them. Copy the versioned
  standard-module directory with a relocated tool bundle. Browser deployments keep
  `slang/`, `src/`, and `wasm/` from the same package build.
- During the bootstrap build, dependencies are compiled from staged sources. Loading
  an emitted dependency alongside the explicit core archive currently fails core-module
  dependency resolution. This build-time workaround does not require source search paths
  in installed applications.
- Bundled modules omit debug IR with `-g0`: the current HGBC debugger cannot consume
  some generic `DebugFunction` records from precompiled dependencies. Application
  debugging still works, but stepping through bundled library bodies is unavailable because
  those modules have no debug information.
  Static-assert messages remain in the diagnostic heading without a library source span.
  Canonical module names (`slang/...`) must match import paths for HGBC dispatch lookup.
- Browser examples use the flag-capture and presentation workarounds described under
  [grid dispatch rough edges](grid-dispatch.md#rough-edges).
- Cooperative grid dispatch and texture mapping derive the linear lane index from
  `SV_GroupThreadID`. Using `SV_GroupIndex` on these specialized generic entry points
  currently fails while converting entry-point parameters to borrow parameters.
- `Grid` expands mixed vector constructors component by component, and `dispatchWaves`
  names its `Wave<Index, subgroupCount>` type before the lambda. These work around HGBC's
  mixed-vector construction gap and accidental capture of a generic value as a runtime
  lambda field. Keep those forms when adapting the implementation.
- Tensors use bindless descriptors, which the current WebGPU backend does not support.
  Tensor and differentiable-tensor sources are bundled for compilation and tooling, but
  executing their allocation/indexing paths requires a supported native backend. For
  browser computation, use ordinary buffers and textures with the utility modules.
