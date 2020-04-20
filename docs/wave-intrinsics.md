Wave Intrinsics
===============

Slang has support for Wave intrinsics introduced to HLSL in SM6.0 and SM6.5. All intrinsics are available on D3D12, and a subset on Vulkan. On CUDA 'WaveMask' intrinsics are introduced which map more directly to the CUDA model of requiring a `mask` of participating lanes. On D3D12 and Vulkan the WaveMask instrinsics can be used, but the mask is effectively ignored. For this to work across targets including CUDA, the mask must be calculated such that it exactly matches that of HLSL defined 'active' lanes, else the behavior is undefined. 

Another wrinkle in compatibility is that on GLSL targets such as Vulkan, the is not built in language support for Matrix versions of Wave intrinsics. Currently this means that Matrix is not a supported type for Wave intrinsics on Vulkan, but may be in the future.

Additional Wave Intrinsics
==========================

T can be scalar, vector or matrix, except on Vulkan which doesn't support Matrix.

```
T WaveBroadcastLaneAt(T value, constexpr int lane);
```

All lanes receive the value specified in lane. Lane must be an active lane, otherwise the result is undefined. 
This is a more restricive version of `WaveReadLaneAt` - which can take a non constexpr lane, *but* must be the same value for all lanes in the warp. Or 'dynamically uniform' as described in the HLSL documentation. 

```
T WaveShuffle(T value, int lane);
```

Shuffle is a less restrictive version of `WaveReadLaneAt` in that it has no restriction on the lane value - it does *not* require the value to be same on all lanes. 

There isn't explicit support for WaveShuffle in HLSL, and for now it will emit `WaveReadLaneAt`. As it turns out for a sizable set of hardware WaveReadLaneAt does work correctly when the lane is not 'dynamically uniform'. This is not necessarily the case for hardware general though, so if targetting HLSL it is important to make sure that this does work correctly on your target hardware.

Our intention is that Slang will support the appropriate HLSL mechanism that makes this work on all hardware when it's available.  

```
void AllMemoryBarrierWithWaveSync();
```

Synchronizes all lanes to the same AllMemoryBarrierWithWaveSync in program flow. Orders all memory accesses such that accesses after the barrier can be seen by writes before.  

```
void GroupMemoryBarrierWithWaveSync();
```

Synchronizes all lanes to the same GroupMemoryBarrierWithWaveSync in program flow. Orders group shared memory accesses such that accesses after the barrier can be seen by writes before.  

Wave Mask Intrinsics
====================

CUDA has a different programming model for inter warp/wave communication based around masks of active lanes. This is because the CUDA programming model allows for divergence that is more granualar than just on program flow, and that there isn't implied reconvergence at the end of a conditional. 

In the future Slang may have the capability to work out the masks required such that the regular HLSL Wave intrinsics work. As it stands there does not appear to be any way to implement the regular Wave intrinsics directly. To work around this problem we introduce 'WaveMask' intrinsics, which are essentially the same as the regular HLSL Wave intrinsics with the first parameter as the WaveMask which identifies the participating lanes. 

The WaveMask intrinsics will work across targets, but *only* if on CUDA targets the mask captures exactly the same lanes as the 'Active' lanes concept in HLSL. If the masks deviate then the behavior is undefined. On non CUDA based targets currently the mask is ignored. This behavior may change on GLSL which has an extension to support a more CUDA like behavior.  

Most of the `WaveMask` functions are identical to the regular Wave intrinsics, but they take a WaveMask as the first parameter, and the intrinsic name starts with `WaveMask`. 

```
WaveMask GetConvergedMask();
```

Gets the mask of lanes which are converged within the Wave. Note that this is *not* the same as Active threads, and may be some subset of that. It is equivalent to the `__activemask()` in CUDA.

On non CUDA targets the the function will return all lanes as active - even though this is not the case. This is 'ok' in so far as on non CUDA targets the mask is ignored. It is *not* okay if the code uses the value other than as a superset of the 'really converged' lanes. For example testing the bit's and changing behavior would likely not work correctly on non CUDA targets. 

```
void AllMemoryBarrierWithWaveMaskSync(WaveMask mask);
```

Same as AllMemoryBarrierWithWaveSync but takes a mask of active lanes to sync with. 

```
void GroupMemoryBarrierWithWaveMaskSync(WaveMask mask);
```

Same as GroupMemoryBarrierWithWaveSync but takes a mask of active lanes to sync with. 
 
 
 