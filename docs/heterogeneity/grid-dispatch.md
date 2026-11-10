# Launch compute kernels

There are a few ways to launch compute kernels in `h-slang`, depending on the complexity.
We recommend the `Grid::dispatch` since it works with lambdas, and provides a natural way to specify the dimensions of the dispatch,
though a raw compute shader dispatch is also supported.

Both methods support specialization of generic shader code.

## Grid::dispatch

`Grid(extent).dispatch(callback)` dispatches one thread per coordinate in extent.
`Grid` is a bundled standard-library utility built on `h-slang`'s [raw dispatch](#raw-dispatch) primitive.
Callbacks use Slang's existing [lambda syntax](https://docs.shader-slang.org/en/latest/external/slang/docs/user-guide/03-convenience-features.html#lambda-expressions-experimental).

<!-- runnable: grid-dispatch -->

```slang
import slang.runtime;

[host]
void main()
{
    let values = RWStructuredBuffer<float>::alloc(1024);
    Grid(1024).dispatch((uint i) => { values[i] = float(i); });

    let output = RWStructuredBuffer<float>::alloc(1024);
    float scale = 2.0;
    Grid(1024).dispatch((uint i) =>
    {
        output[i] = scale * values[i] + 1.0;
    });
    print(output[256]); // 513.000000
}
```

`main` executes on the CPU; both lambdas execute on the GPU. The second dispatch consumes the first's results. Host indexing waits only when the host needs the answer.

### Multi-dimensional dispatch & workgroup size.

The `extent` parameter can be a `vector<uint>` for multi-dimensional dispatch.Workgroup size can be specified (optionally) as compile-time specialization of the dispatch.

| Call                                             | Group size  | Callback coordinates     |
| ------------------------------------------------ | ----------- | ------------------------ |
| `Grid(1024).dispatch(...)`                       | 64 × 1 × 1  | `uint`, 0…1023           |
| `Grid(1024).dispatch<128>(...)`                  | 128 × 1 × 1 | `uint`, 0…1023           |
| `Grid(uint2(32, 32)).dispatch<8, 8>(...)`        | 8 × 8 × 1   | `uint2`, (0, 0)…(31, 31) |
| `Grid(uint3(16, 16, 16)).dispatch<4, 4, 4>(...)` | 4 × 4 × 4   | `uint3`, each axis 0…15  |

<!-- runnable: grid-2d canvas=16:9 -->

```slang
import slang.runtime;

[host]
void main(Window w)
{ //!FOLD
    let surface = createSurface<TextureFormat::rgba8Unorm>(w);
    let presented = acquireSurfaceTexture(surface);
    uint width, height;
    presented.GetDimensions(width, height);
    let size = uint2(width, height);
    let image = draw(size);
    blitTexture2D(presented, image);
    presentSurface(surface);
}

RWTexture2D<float4> draw(uint2 resolution)
{
    let image = RWTexture2D<float4>::alloc(resolution.x, resolution.y);
    Grid(resolution).dispatch<8, 8>((uint2 p) =>
    {
        float2 uv = (float2(p) + 0.5) / float2(resolution);
        image[p] = float4(uv.x, uv.y, 0.35, 1.0);
    });
    return image;
}
```

The callback receives a global coordinate; you choose how to flatten it. Partial groups
skip padding coordinates, so `Grid(1000).dispatch<128>(...)` calls the callback exactly
1,000 times. Any zero dimension queues no work.

### Rough edges

<!-- (should follow up but not really important enough to include in the docs)
**Host vector conversions.** HGBC does not yet support general vector element-type
casts. Construct host vectors from matching scalar types, such as
`uint2(0u, 0u)`, and perform vector conversions such as `float2(size)` inside the
device callback. The canvas example follows this workaround.

**Browser presentation.** Use `TextureFormat::rgba8Unorm` for the canvas surface;
the current WebGPU backend does not support explicit sRGB surface requests. The
examples compute into `RWTexture2D<float4>` and blit to the surface. That path
currently requires the adapter's `float32-filterable` feature. Keep acquired surface
images local to the drawing helper so they are released before `resizeSurface`.
-->

**Boolean captures on WebGPU.** WGSL does not permit `bool` in uniform data.

## Raw dispatch

Use `dispatch(kernel, groupCount)(arguments...)` when you want to call a shader directly.
Here **groupCount counts workgroups**, so the shader code is responsible for handling bounds checks.

<!-- runnable: raw-dispatch -->

```slang
[shader("compute")]
[numthreads(64, 1, 1)]
void fill(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<uint> output, uint count)
{
    if (tid.x < count)
        output[tid.x] = tid.x;
}

[host]
void main()
{
    let output = RWStructuredBuffer<uint>::alloc(1000);
    dispatch(fill, uint3(16, 1, 1))(output, 1000);
    print(output[999]); // 999
}
```

### Specialized dispatch: call generic shaders

Types and compile-time values specialize the kernel; resource and scalar arguments remain
runtime bindings. This is a separate, complete example:

<!-- runnable: specialized-dispatch -->

```slang
[shader("compute")]
[numthreads(64, 1, 1)]
void fillValue<T : __BuiltinArithmeticType, let Value : int>(
    uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<T> output)
{
    output[tid.x] = T(Value);
}

[host]
void main()
{
    let output = RWStructuredBuffer<float>::alloc(64);
    dispatch(fillValue<float, 7>, uint3(1, 1, 1))(output);
    print(output[0]); // 7.000000
}
```

## Rough edges

**Push constant limits.**
Non-resource arguments are passed via push constant by default. Large structs or arrays can exceed the backend's push/root constant budget. Pack large parameter data into a buffer and pass the buffer instead:

<!-- runnable: packed-captures -->

```slang
import slang.runtime;

[host]
void main()
{
    let cpuParameters = RuntimeArray<float>::alloc(256);
    for (uint i = 0; i < 256; ++i)
        cpuParameters[i] = float(i);
    let parameters = RWStructuredBuffer<float>::alloc(256);
    parameters[range(0, 256)].copyFrom(cpuParameters);
    let output = RWStructuredBuffer<float>::alloc(1024);
    Grid(1024).dispatch((uint i) => { output[i] = parameters[i % 256]; });
    print(output[255]); // 255.000000
}
```

At the moment, there is no explicit support for constant buffers, though support for this is on the roadmap.

**`Grid::dispatch` assumes independent invocations.** For multi-dimensional dispatch, coordinate execution order is unspecified (e.g. row-major vs. col-major).
This may additionally skip padding lanes and cause workgroup divergence. Workgroup-wide barriers and collectives should prefer using raw dispatch (#raw-dispatch) for now.

## Behind the scenes

**Resource detection, pipelines, and binding.** Slang identifies the device entry point,
specializes its code, and describes its parameter layout. HGVM finds resources in the
arguments and closure, creates or reuses a compatible pipeline, and binds the values.
Each host argument retains its original shader parameter index. Empty arguments, including
callbacks with no captures, consume no GPU bindings; removing an empty host argument during
compilation leaves the remaining parameter indices unchanged. Reflection determines the bytes
and resource bindings needed by each parameter.
`Grid::dispatch` adds coordinate generation and padding checks around this raw mechanism.

**Dispatch buffering.** Calls record GPU work into command batches rather than blocking
for each kernel. The VM retains resources and orders dependent operations. A host readback
flushes pending work and synchronizes; avoid inserting one between every pair of kernels.

See the standalone [grid dispatch example](../../examples/host-compute/grid-dispatch.slang)
and [Grid implementation](../../source/standard-modules/host/runtime/compute/grid.slang).
