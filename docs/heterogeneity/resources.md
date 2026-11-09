# Resource Types

`h-slang` currently supports a subset of Slang's intrinsic resource types: `RWStructuredBuffer<T>`, `RWTexture2D<T>`, and `RaytracingAccelerationStructure` (TLAS). Hardware BLAS objects are exposed as
`HWRaytracingContext<...>.BottomLevelAccelerationStructure`.

## Allocation & Usage

Allocate in `[host]` code. Buffers support single-element indexing on both host and device;
texture indexing runs in device callbacks. Buffers and textures use `import slang.runtime;`.
Acceleration allocation and builds additionally require `import slang.pipeline_rt;`.
In the table, `RTContext` is an alias for `HWRaytracingContext<...>` with your ray types.

| Type                                     | Allocation                                                          | Device use                              |
| ---------------------------------------- | ------------------------------------------------------------------- | --------------------------------------- |
| `RWStructuredBuffer<T>`                  | `RWStructuredBuffer<float>::alloc(1024)`                            | `buffer[i]`                             |
| `RWTexture2D<T>`                         | `RWTexture2D<float4>::alloc(640, 480)`                              | `image[uint2(x, y)]`                    |
| `RTContext.AccelerationStructure` (TLAS) | `RTContext.AccelerationStructure::alloc()` then `.build(instances)` | `context.traceRay(scene, ray, payload)` |

The TLAS references BLAS instances. See the complete
[triangle scene build](ray-tracing.md#example-render-a-triangle) for vertex/index uploads,
BLAS creation, and instance construction.

### Bindless access

All three primitives can be represented by descriptor handles. Handles retain their
resource; dereference them in device code. For example:

```slang
import slang.runtime;

[host]
void main()
{
    let values = RWStructuredBuffer<uint>::alloc(4);
    let handle = values.getDescriptorHandle();
    Grid(4).dispatch((uint i) => { handle[i] = i + 10; });
    print(values[3]); // 13
}
```

Textures use `textureHandle[p]`; acceleration structures use `*sceneHandle`.
Ray-tracing callbacks [require explicit handles](ray-tracing.md#rough-edges).

## Resource indexing and transfers

### Single elements

Host buffer indexing transfers one element. `StructuredBuffer<T>` supports reads;
`RWStructuredBuffer<T>` also supports writes and compound assignments.

<!-- runnable: host-element-access -->

```slang
[host]
void main()
{
    let values = RWStructuredBuffer<float>::alloc(4);
    values[2] = 7.0;       // Upload one element.
    values[2] += 1.0;      // Read, modify, and upload one element.
    print(values[2]);      // 8.000000; wait for GPU work and read one element.
}
```

For struct fields or vector components, edit a temporary and assign the whole element:

```slang
// Inside a host function, with RWStructuredBuffer<float3> positions:
var position = positions[2];
position.x = 7.0;
positions[2] = position;
```

In a GPU callback, `buffer[i]` accesses device memory directly; field writes and
atomic operations keep their usual shader semantics. Texture indexing also belongs
in device callbacks.

### Range views

Import `slang.runtime`. `range(begin, end)` selects `[begin, end)` in element units:

```slang
let buffer = RWStructuredBuffer<float>::alloc(8);
let view = buffer[range(2, 6)]; // BufferView of four elements; no copy or readback.
print(view.getCount());        // 4
```

A view describes a sub-range of an existing buffer. Creating or copying
a view does not transfer data. The same getter works on `StructuredBuffer<T>`.

`h-slang` does not currently enforce borrow checking for buffer views; it is the programmer's responsibility to avoid overlapping mutable views.

### Upload and read back

`.copyFrom(array)` uploads an equally sized `RuntimeArray<T>`. `.host()` allocates an
independent host array and waits for the selected contents:

<!-- runnable: upload-buffer -->

```slang
import slang.runtime;

[host]
void main()
{
    let cpu = RuntimeArray<float>::alloc(2);
    cpu[0] = 3.0;
    cpu[1] = 4.0;
    let gpu = RWStructuredBuffer<float>::alloc(4);
    let view = gpu[range(1, 3)];
    view.copyFrom(cpu);          // One upload of two elements.

    let snapshot = view.host();  // One readback of two elements.
    snapshot[1] = 9.0;
    print(gpu[2]);               // 4.000000; snapshot edits do not change the GPU.
    view.copyFrom(snapshot);     // Explicitly upload the edited values.
    print(gpu[2]);               // 9.000000
}
```

Each `.host()` call makes a new snapshot. It does not map device memory or arrange
automatic writeback.

### Copy between buffers

Assign a view to an equally sized range, or call `.copyFrom(otherView)`:

<!-- runnable: buffer-range-copy -->

```slang
import slang.runtime;

[host]
void main()
{
    let source = RWStructuredBuffer<uint>::alloc(2);
    source[0] = 17;
    source[1] = 23;
    let destination = RWStructuredBuffer<uint>::alloc(6);

    destination[range(1, 3)] = source[range(0, 2)]; // Device copy; no host round trip.
    destination[range(3, 5)].copyFrom(source[range(0, 2)]);
    let result = destination[range(1, 5)].host();
    for (uint i = 0; i < uint(result.getCount()); ++i)
        print(result[i]); // 17, 23, 17, 23 (one per line).
}
```

Use `.copyFrom()` to copy from a read-only buffer view too:

```slang
// readOnlySource is a populated StructuredBuffer<float>.
let sourceView = readOnlySource[range(0, 4)];
let cpu = sourceView.host();
destination[range(4, 8)].copyFrom(sourceView);
```

Assigning a **view variable** just replaces the view; it does not copy elements:

```slang
var a = destination[range(0, 2)];
let b = source[range(0, 2)];
a.copyFrom(b); // Copy source's elements into destination.
a = b;         // a now refers to source's range; no element copy.
```

### Explicit offset copies

`copy(destination, destinationOffset, source, sourceOffset, count)` remains available
for partial host arrays and existing readback storage. Use range assignment for the
device-to-device step:

```slang
let cpu = RuntimeArray<float>::alloc(8);
let gpu = RWStructuredBuffer<float>::alloc(8);
let other = RWStructuredBuffer<float>::alloc(8);
cpu[3] = 7.0;
cpu[4] = 8.0;
copy(gpu, 1, cpu, 3, 2);   // Upload cpu[3..5) into gpu[1..3).
other[range(4, 6)] = gpu[range(1, 3)]; // Device copy.
copy(cpu, 0, other, 4, 2); // Read back into existing cpu[0..2).
```

This also supports initializing a `StructuredBuffer<T>` before shader use.

### Tensor coordinates

Tensor host subscripts transfer one element and check coordinates against the shape:

```slang
import slang.runtime;
import slang.tensor;

[host]
void main()
{
    let image = Tensor<float, 2>(uint2(4, 3));
    image.fill(0.0);
    image[uint2(2, 1)] = 7.0;
    print(image[uint2(2, 1)]); // 7.000000
}
```

Use [map and reduce](#map-and-reduce) to transform or aggregate many elements on the
GPU. Multidimensional/strided slices are not exposed yet. Tensor execution requires
a native backend with bindless support.

**Bounds and lengths.** Ranges must satisfy `begin <= end <= buffer.getCount()`.
Empty ranges are allowed. `.copyFrom()` and range assignment require equal lengths;
they do not truncate or resize the destination. Range assignment accepts a matching
writable-buffer view; use `.copyFrom()` for a host array or read-only source view.

**Device copies.** Source and destination must have identical physical element
layouts. Overlapping regions of the same buffer are rejected; copying a range onto
itself is a no-op.

**Synchronization and layout.** Host element reads and `.host()` wait for the needed
GPU work. Uploads and device copies retain command ordering. Typed host/device copies
convert supported element layouts automatically. Views currently cover structured
buffers; they do not expose host pointers into GPU storage.

## Tensor<T, Rank>

The bundled `Tensor` combines GPU storage with a shape. Ranks 1–4 are supported.
Copying a tensor shares storage; it does not copy its elements.

```slang
import slang.runtime;
import slang.tensor;

[host]
void main()
{
    let image = Tensor<float, 2>(uint2(4, 3));
    image.fill(2.0);
    let doubled = image.map((uint2 p, float value) => value * 2.0);
    let total = doubled.reduce(0.0, (float a, float b) => a + b);
    print(total[0]); // 48.000000
}
```

### Layout and strided access

The first axis is contiguous. `tensor[uint2(x, y)]` addresses `x + width * y`;
`tensor[uint3(x, y, z)]` addresses `x + width * (y + height * z)`.
Those strides are derived from the shape. **Arbitrary strided views/slices are not
currently exposed by this Tensor implementation.**

See [resource indexing and transfers](#resource-indexing-and-transfers) for host
coordinate access.

`reserve`/`resize` can grow scratch storage, discarding contents when allocation grows.
Within capacity, resizing reinterprets existing flat storage. A captured tensor keeps
its previous buffer and shape snapshot.

### map and reduce

Both operations allocate result tensors. `map` receives each element's coordinate
and value; `reduce` takes an identity and a combining function. Inside a host function
with `import slang.tensor;`:

```slang
let input = Tensor<float, 1>(4u);
input.fill(3.0);
let output = input.map((uint1 i, float value) => value * 2.0);
let total = output.reduce(0.0, (float a, float b) => a + b);
print(total[0]); // 24.000000
```

Reduction combines must be associative; floating-point results can differ from a
serial sum. Tensor `reduce(identity, combine)` uses `identity` for an empty input.

### dmap and differentiable reduction

Import `slang.tensor.differentiable` for derivative-aware operations:

| Operation                                   | Result                                                       |
| ------------------------------------------- | ------------------------------------------------------------ |
| `input.dmap((uint1 i, float x) ~ => x * x)` | New differentiable tensor                                    |
| `input.dmapTo(output, transform)`           | Write a distinct output tensor                               |
| `input.dsum()` / `input.dsumTo(output)`     | Differentiable float sum                                     |
| `dreduce`                                   | No general operation with this name yet; use `dsum` for sums |

See [chained differentiable compute calls](automatic-differentiation.md) for an end-to-end
backward pass, gradient allocation, and output.

## Rough edges

**Conservative resource access.** Treat general writable resource use as read/write for
dependency and performance planning; precise per-kernel access flags is still work-in-progress.
Some read-only resource types are provided (e.g., `StructuredBuffer`), but only partially.

**Allocation flags.** Buffer/texture helpers largely choose fixed usage flags (which are much broader than generally necessary). Broader flags and conservative synchronization can be considerably slower than carefully tuned native allocations. More allocation controls and access analysis are areas for future improvement.

**External values.** Bindless use inside Slang is broader than the current external API:
only supported resource types can cross into Python/JS/C++. Check the
[interop resource limits](interoperability.md#rough-edges).

Tensor execution currently requires bindless descriptors, which are only supported on newer graphics APIs (D3D12, Vulkan, CUDA, Metal) & hardware. One notable exception is WebGPU.

Sources: [Tensor and tensor operations](../../source/standard-modules/host/tensor.slang),
[differentiable tensors](../../source/standard-modules/host/tensor/differentiable.slang).
