# Host functions

`[host]` marks code that the VM executes on the CPU. Unlike device code, host code can allocate resources,
dispatch GPU work, and use host-specific APIs like window creation & input handling.

<!-- runnable: host-answer -->

```slang
import slang.runtime;

// A host function can launch GPU work and return its result.
[host]
public uint answer()
{
    let values = RWStructuredBuffer<uint>::alloc(4);
    Grid(4).dispatch((uint i) => { values[i] = 40 + i; });

    // Waits for the dispatch, then reads back one element.
    return values[2];
}

[host]
void main()
{
    print(answer()); // 42
}
```

`answer` runs on the host; the lambda runs on the GPU. Host indexing waits for the result.
Call concrete host exports from [Python, C++, or JavaScript](interoperability.md).

## Allocate buffers and textures

Allocate resources inside a host function:

<!-- runnable: allocate-resources -->

```slang
import slang.runtime;

[host]
void main()
{
    // GPU memory; nothing is copied from the CPU.
    let values = RWStructuredBuffer<float>::alloc(1024);
    let image = RWTexture2D<float4>::alloc(640, 480);
    print("Allocated one buffer and one texture.");
}
```

Allocation reserves storage; initialize contents before reading. See
[resource indexing and transfers](resources.md#resource-indexing-and-transfers) for
single-element access, range views, uploads, readbacks, and buffer-to-buffer copies.

## Surface waits and input timing

For an interactive native window, call `waitForSurface(surface)` before pumping the next frame's
input with `processWindowEvents(window)` and `pollInputEvent(window)`. The wait reserves an
available presentation image internally. Input handling and offscreen rendering can then submit
GPU work, including picking and readbacks. Call `tryAcquireSurfaceTexture` only when that work
is finished, immediately before the final blit and `tryPresentSurface`.

`waitForSurface` returns `false` if the drawable area is unavailable or the swapchain needs an
explicit `resizeSurface`. Repeated waits reuse the reservation, and resizing or releasing the
surface discards it. Acquisition also works without a preceding wait. This is an image
availability wait; it does not promise a particular display time or presentation queue depth.

Use `surface.getDimensions()` to size the initial offscreen target without acquiring an image.
It returns the configured image dimensions, or zero dimensions for an unconfigured surface.
Keep acquired textures local to a drawing helper so their references are released before resize.

In the browser, `processWindowEvents` already yields to the browser's frame scheduler.
`waitForSurface` checks readiness there; canvas acquisition stays inside final presentation so
a suspended computation cannot retain an expired canvas texture.

## Behind the scenes

**Reference counting.** Resource values retain their allocations. Copies and closures keep
them alive, and queued GPU commands retain what they use until completion. You can return a
buffer from a helper without managing a separate owner object.

**Automatic barriers.** The VM tracks resource use and inserts the transitions and ordering
needed between uploads, dispatches, draws, and readbacks. This does not resolve races between
threads inside a shader.

**Allocation pooling.** The `h-slang` VM maintains a pool of allocated buffers. Released buffers aren't freed immediately unless evicted due to memory requirements (or VM tear-down), and are instead cached against the descriptor (size, stride, format, access flags). A subsequent allocation with an exact matching descriptor can reuse a cached buffer, avoiding a fresh native allocation.

This leverages the fact the majority of subsequent allocation requests of the next frame/iteration usually have the same descriptor.

A follow-up task is to implement more sophisticated pooling: if an exact match isn't available, use a closest match (e.g. closest larger buffer)

See [resource types and limitations](resources.md) and the
[buffer pool implementation](../../source/slang-hgvm/slang-hgvm-buffer-pool.h).

## Host and device arithmetic

Host code follows the same arithmetic rules as HLSL device code, so a helper called from both
sides gives the same answer:

- A shift amount is masked to the value's bit width: `x << 32` is `x` and `x << 33` is `x << 1`.
  `>>` is arithmetic for signed values and logical for unsigned values.
- A float-to-integer conversion truncates toward zero, saturates to the destination range, and
  converts NaN to zero: `int(3e9)` is `2147483647` and `uint(-1.5)` is `0`.
- `asuint`, `asint`, and `asfloat` keep every bit, and `abs(-0.0)` is `+0.0`.

GPU backends add a few differences of their own:

- **CUDA shifts.** Slang's CUDA output does not mask shift amounts, so on CUDA a shift by 32 or
  more returns `0` instead of matching host code. Keep shift amounts below the bit width in code
  that must agree on every backend.
- **Subnormals.** D3D12 and most Vulkan drivers flush subnormal floats to zero; host code keeps
  them.
- **NaN payloads.** GPU arithmetic may return a different NaN bit pattern than host code; both
  are NaN.
- **Half round trips.** The NVIDIA Vulkan driver can drop a `float(half(x))` round trip, so an
  out-of-range value stays finite on the GPU while host code rounds it to infinity.
- **Approximate math.** Device `rsqrt`, `rcp`, `normalize`, and similar functions may differ from
  host code by a few units in the last place.
