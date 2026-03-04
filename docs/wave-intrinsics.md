
Wave Intrinsics
===============

Slang has support for Wave intrinsics introduced to HLSL in [SM6.0](https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12) and [SM6.5](https://github.com/microsoft/DirectX-Specs/blob/master/d3d/HLSL_ShaderModel6_5.md). All intrinsics are available on D3D12 and Vulkan.

On GLSL targets such as Vulkan wave intrinsics map to ['subgroup' extension] (https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt).  Vulkan supports a number of masked wave operations through `SPV_NV_shader_subgroup_partitioned` that are not supported by HLSL.

There is no subgroup support for Matrix types, and currently this means that Matrix is not a supported type for Wave intrinsics on Vulkan, but may be in the future.


Also introduced are some 'non standard' Wave intrinsics which are only available on Slang. All WaveMask intrinsics are non standard. Other non standard intrinsics expose more accurately different behaviours which are either not distinguished on HLSL, or perhaps currently unavailable. Two examples would be `WaveShuffle` and `WaveBroadcastLaneAt`. 

There are three styles of wave intrinsics...

## WaveActive

The majority of 'regular' HLSL Wave intrinsics which operate on implicit 'active' lanes. 

In the [DXC Wiki](https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics) active lanes are described as

> These intrinsics are dependent on active lanes and therefore flow control. In the model of this document, implementations
> must enforce that the number of active lanes exactly corresponds to the programmer’s view of flow control.
 
In practice this appears to imply that the programming model is that all lanes operate in 'lock step'. That the 'active lanes' are the lanes doing processing at a particular point in the control flow. On some hardware this may match how processing actually works. There is also a large amount of hardware in the field that doesn't follow this model, and allows lanes to diverge and not necessarily on flow control. On this style of hardware Active intrinsics may act to also converge lanes to give the appearance of 'in step' ness. 
 
## WaveMask

The WaveMask intrinsics take an explicit mask of lanes to operate on, in the same vein as CUDA. Requesting data from a from an inactive lane, can lead to undefined behavior, that includes locking up the shader. The WaveMask is an integer type that can hold the maximum amount of active lanes for this model - currently 32. In the future the WaveMask type may be made an opaque type, but can largely be operated on as if it is an integer.

Using WaveMask intrinsics is generally more verbose and prone to error than the 'Active' style, but it does have a few advantages

* It works across all supported targets - including CUDA (currently WaveActive intrinics do not)
* Gives more fine control
* Might allow for higher performance (for example it gives more control of divergence)
* Maps most closely to CUDA

For this to work across targets including CUDA, the mask must be calculated such that it exactly matches that of HLSL defined 'active' lanes, else the behavior is undefined.

On D3D12 and Vulkan the WaveMask intrinsics can be used, but the mask may be ignored depending on target's support for partitioned/masked wave intrinsics. SPIRV provides support for a wide variety of operations through the `SPV_NV_shader_subgroup_partitioned` extension while HLSL only provides a small subset of operations through `WaveMultiPrefix*` intrinsics. The difference between Slang's `WaveMask`  and these targets' partitioned wave intrinsics is that they accept a `uint4` mask instead of a `uint` mask. `WaveMask*` intrinsics effectively gets translated  to `WaveMulti*` intrinsics when targeting SPIRV/GLSL and HLSL. Please consult [Wave Multi Intrinsics](#wave-multi-intrinsics) for more details, including what masked operations are supported by each target.


The WaveMask intrinsics are a non standard Slang feature, and may change in the future. 

```
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // It is the programmers responsibility to determine the initial mask, and that is dependent on the launch
    // It's common to launch such that all lanes are active - with CUDA this would mean 32 lanes. 
    // Here the launch only has 4 lanes active, and so the initial mask is 0xf.
    const WaveMask mask0 = 0xf;
    
    int idx = int(dispatchThreadID.x);
    
    int value = 0;
    
    // When there is a conditional/flow control we typically need to work out a new mask.
    // This can be achieved by calling WaveMaskBallot with the current mask, and the condition
    // used in the flow control - here the subsequent 'if'.
    const WaveMask mask1 = WaveMaskBallot(mask0, idx == 2);
    
    if (idx == 2)
    {
        // At this point the mask is `mask1`, although no WaveMask intrinsics are used along this path, 
        // so it's not used.
    
        // diverge
        return;
    }
    
    // If we get here, the active lanes must be the opposite of mask1 (because we took the other side of the condition), but cannot include
    // any lanes which were not active before. We can calculate this as mask0 & ~mask1.
    
    const WaveMask mask2 = mask0 & ~mask1;
    
    // mask2 holds the correct active mask to use with WaveMaskMin
    value = WaveMaskMin(mask2, idx + 1);
    
    // Write out the result
    outputBuffer[idx] = value;
}
```

Many of the nuances of writing code in this way are discussed in the [CUDA documentation](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#warp-vote-functions).

The above example written via the regular intrinsics is significantly simpler, as we do not need to track 'active lanes' in the masks. 

```
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);
    
    int value = 0;
    
    if (idx == 2)
    {    
        // diverge
        return;
    }
    
    value = WaveActiveMin(idx + 1);
    
    // Write out the result
    outputBuffer[idx] = value;
}
```
## WaveMulti

The standard 'Multi' intrinsics were added to HLSL is SM 6.5 and are available in SPIRV through `SPV_NV_shader_subgroup_partitioned`, they can specify a mask of lanes via uint4. SPIRV provide non-prefix (reduction) and prefix (scan) intrinsics for arithmetic and min/max operations, while HLSL only provides a subset of these, namely exclusive prefix arithmetic operations.


Standard Wave intrinsics
=========================

The Wave Intrinsics supported on Slang are listed below. Note that typically T generic types also include vector and matrix forms. 

```
// Lane info

uint WaveGetLaneCount();

uint WaveGetLaneIndex();

bool WaveIsFirstLane();

// Ballot

bool WaveActiveAllTrue(bool condition);

bool WaveActiveAnyTrue(bool condition);

uint4 WaveActiveBallot(bool condition);

uint WaveActiveCountBits(bool value);

// Across Lanes

T WaveActiveBitAnd<T>(T expr);

T WaveActiveBitOr<T>(T expr);

T WaveActiveBitXor<T>(T expr);

T WaveActiveMax<T>(T expr);

T WaveActiveMin<T>(T expr);

T WaveActiveProduct<T>(T expr);

T WaveActiveSum<T>(T expr);

bool WaveActiveAllEqual<T>(T value);

// Prefix

T WavePrefixProduct<T>(T expr);

T WavePrefixSum<T>(T expr);

// Communication

T WaveReadLaneFirst<T>(T expr);

T WaveReadLaneAt<T>(T value, int lane);

// Prefix

uint WavePrefixCountBits(bool value);

// Shader model 6.5 stuff
// https://github.com/microsoft/DirectX-Specs/blob/master/d3d/HLSL_ShaderModel6_5.md

uint4 WaveMatch<T>(T value);

uint WaveMultiPrefixCountBits(bool value, uint4 mask);

T WaveMultiPrefixBitAnd<T>(T expr, uint4 mask);

T WaveMultiPrefixBitOr<T>(T expr, uint4 mask);

T WaveMultiPrefixBitXor<T>(T expr, uint4 mask);

T WaveMultiPrefixProduct<T>(T value, uint4 mask);

T WaveMultiPrefixSum<T>(T value, uint4 mask);
```

Non Standard Wave Intrinsics
============================

The following intrinsics are not part of the HLSL Wave intrinsics standard, but were added to Slang for a variety of reasons. Within the following signatures T can be scalar, vector or matrix, except on Vulkan which doesn't (currently) support Matrix.

```
T WaveBroadcastLaneAt<T>(T value, constexpr int lane);

T WaveShuffle<T>(T value, int lane);

uint4 WaveGetActiveMulti();

uint4 WaveGetConvergedMulti();

uint WaveGetNumWaves();  // Number of waves in workgroup (equivalent to GLSL gl_NumSubgroups)

// Prefix (exclusive scan operations, equivalent to GLSL subgroupExclusive*)

T WavePrefixMin<T>(T expr);   // Exclusive prefix minimum

T WavePrefixMax<T>(T expr);   // Exclusive prefix maximum

T WavePrefixBitAnd<T>(T expr);  // Exclusive prefix bitwise AND

T WavePrefixBitOr<T>(T expr);   // Exclusive prefix bitwise OR

T WavePrefixBitXor<T>(T expr);  // Exclusive prefix bitwise XOR

// Barriers 

void AllMemoryBarrierWithWaveSync();

void GroupMemoryBarrierWithWaveSync();
```

## Description

```
T WaveBroadcastLaneAt<T>(T value, constexpr int lane);
```

All lanes receive the value specified in lane. Lane must be an active lane, otherwise the result is undefined. 
This is a more restrictive version of `WaveReadLaneAt` - which can take a non constexpr lane, *but* must be the same value for all lanes in the warp. Or 'dynamically uniform' as described in the HLSL documentation.

```
T WaveShuffle<T>(T value, int lane);
```

Shuffle is a less restrictive version of `WaveReadLaneAt` in that it has no restriction on the lane value - it does *not* require the value to be same on all lanes. 

There isn't explicit support for WaveShuffle in HLSL, and for now it will emit `WaveReadLaneAt`. As it turns out for a sizable set of hardware WaveReadLaneAt does work correctly when the lane is not 'dynamically uniform'. This is not necessarily the case for hardware general though, so if targeting HLSL it is important to make sure that this does work correctly on your target hardware.

Our intention is that Slang will support the appropriate HLSL mechanism that makes this work on all hardware when it's available.  

```
void AllMemoryBarrierWithWaveSync();
```

Synchronizes all lanes to the same AllMemoryBarrierWithWaveSync in program flow. Orders all memory accesses such that accesses after the barrier can be seen by writes before.  

```
void GroupMemoryBarrierWithWaveSync();
```

Synchronizes all lanes to the same GroupMemoryBarrierWithWaveSync in program flow. Orders group shared memory accesses such that accesses after the barrier can be seen by writes before.  


Wave Rotate Intrinsics
======================

These intrinsics are specific to Slang and were added to support the subgroup rotate functionalities provided by SPIRV (through the `GroupNonUniformRotateKHR` capability), GLSL (through the `GL_KHR_shader_subgroup_rotate
` extension), and Metal.

```
// Supported on SPIRV, GLSL, and Metal targets.
T WaveRotate(T value, uint delta);

// Supported on SPIRV and GLSL targets.
T WaveClusteredRotate(T value, uint delta, constexpr uint clusterSize);
```

Wave Multi Intrinsics
======================

`WaveMulti` intrinsics take an explicit  `uint4` mask of lanes to operate on. They correspond to the subgroup partitioned intrinsics provided by `SPV_NV_shader_subgroup_partitioned`  and the `WaveMultiPrefix*` intrinsics provided by HLSL SM 6.5.  HLSL's `WaveMulti*` intrinsics only provide operations for exclusive prefix arithmetic operations, while Vulkan's `SPV_NV_shader_subgroup_partitioned` provides operations for both inclusive/exclusive prefix (scan) and non-prefix (reduction) arithmetic and min/max operations. 

Slang adds new `WaveMulti*` intrinsics in addition to HLSL's  `WaveMultiPrefix*` to allow generating all partitioned intrinsics supported in SPIRV. The new, non-standard HLSL,  `WaveMulti*` intrinsics are only supported when targeting SPIRV, GLSL and CUDA. The inclusive variants of HLSL's `WaveMultiPrefix*` intrinsics are emulated by Slang by performing an additional operation in the current invocation. Metal and WGSL targets do not support `WaveMulti` intrinsics.
```
// Across lane ops. These are only supported when targeting SPIRV, GLSL and CUDA.

T WaveMultiSum(T value, uint4 mask);

T WaveMultiProduct(T value, uint4 mask);

T WaveMultiMin(T value, uint4 mask);

T WaveMultiMax(T value, uint4 mask);

T WaveMultiBitAnd(T value, uint4 mask);

T WaveMultiBitOr(T value, uint4 mask);

T WaveMultiBitXor(T value, uint4 mask);


// Prefix arithmetic operations. Supported when targeting SPIRV, GLSL, CUDA and HLSL.
// In addition to these non-HLSL standard intrinsics are the standard `WaveMultiPrefix*`
// intrinsics provided by SM 6.5, detailed in the `Standard Wave Intrinsics` section.

T WaveMultiPrefixInclusiveSum(T value, uint4 mask);

T WaveMultiPrefixInclusiveProduct(T value, uint4 mask);

T WaveMultiPrefixInclusiveBitAnd(T value, uint4 mask);

T WaveMultiPrefixInclusiveBitOr(T value, uint4 mask);

T WaveMultiPrefixInclusiveBitXor(T value, uint4 mask);

T WaveMultiPrefixExclusiveSum(T value, uint4 mask);

T WaveMultiPrefixExclusiveProduct(T value, uint4 mask);

T WaveMultiPrefixExclusiveBitAnd(T value, uint4 mask);

T WaveMultiPrefixExclusiveBitOr(T value, uint4 mask);

T WaveMultiPrefixExclusiveBitXor(T value, uint4 mask);


// Prefix min/max operations. Supported when targeting SPIRV and GLSL.

T WaveMultiPrefixInclusiveMin(T value, uint4 mask);

T WaveMultiPrefixInclusiveMax(T value, uint4 mask);

T WaveMultiPrefixExclusiveMin(T value, uint4 mask);

T WaveMultiPrefixExclusiveMax(T value, uint4 mask);
```


Wave Mask Intrinsics
====================

CUDA has a different programming model for inter warp/wave communication based around masks of active lanes. This is because the CUDA programming model allows for divergence that is more granualar than just on program flow, and that there isn't implied reconvergence at the end of a conditional. 

In the future Slang may have the capability to work out the masks required such that the regular HLSL Wave intrinsics work. As it stands there does not appear to be any way to implement the regular Wave intrinsics directly. To work around this problem we introduce 'WaveMask' intrinsics, which are essentially the same as the regular HLSL Wave intrinsics with the first parameter as the WaveMask which identifies the participating lanes.

The WaveMask intrinsics will work across targets, but *only* if on CUDA targets the mask captures exactly the same lanes as the 'Active' lanes concept in HLSL. If the masks deviate then the behavior is undefined. On non CUDA based targets currently the mask *may* be ignored depending on the intrinsics supported by the target.

Most of the `WaveMask` functions are identical to the regular Wave intrinsics, but they take a WaveMask as the first parameter, and the intrinsic name starts with `WaveMask`. Also note that the `WaveMask` functions are introduced in Slang before the `WaveMulti` intrinsics, and they effectively function the same other than the mask width in bits (`uint` vs `uint4`). The `WaveMulti` intrinsics map closer to SPIRV and HLSL, and are recommended to be used over `WaveMask` intrinsics whenever possible. We plan to deprecate the `WaveMask` intrinsics some time in the future.

```
WaveMask WaveGetConvergedMask();
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
 
The intrinsics that make up the Slang `WaveMask` extension. 
 
```
// Lane info

WaveMask WaveGetConvergedMask();

WaveMask WaveGetActiveMask();

bool WaveMaskIsFirstLane(WaveMask mask);

// Lane masks (equivalent to GLSL gl_SubgroupXxMask)

uint4 WaveGetLaneEqMask();  // Mask with only current lane's bit set

uint4 WaveGetLaneGeMask();  // Mask with bits set for lanes >= current lane

uint4 WaveGetLaneGtMask();  // Mask with bits set for lanes > current lane

uint4 WaveGetLaneLeMask();  // Mask with bits set for lanes <= current lane

uint4 WaveGetLaneLtMask();  // Mask with bits set for lanes < current lane

// Ballot

bool WaveMaskAllTrue(WaveMask mask, bool condition);

bool WaveMaskAnyTrue(WaveMask mask, bool condition);

WaveMask WaveMaskBallot(WaveMask mask, bool condition);

WaveMask WaveMaskCountBits(WaveMask mask, bool value);

WaveMask WaveMaskMatch<T>(WaveMask mask, T value);

// Barriers

void AllMemoryBarrierWithWaveMaskSync(WaveMask mask);

void GroupMemoryBarrierWithWaveMaskSync(WaveMask mask);

// Across lane ops

T WaveMaskBitAnd<T>(WaveMask mask, T expr);

T WaveMaskBitOr<T>(WaveMask mask, T expr);

T WaveMaskBitXor<T>(WaveMask mask, T expr);

T WaveMaskMax<T>(WaveMask mask, T expr);

T WaveMaskMin<T>(WaveMask mask, T expr);

T WaveMaskProduct<T>(WaveMask mask, T expr);

T WaveMaskSum<T>(WaveMask mask, T expr);

bool WaveMaskAllEqual<T>(WaveMask mask, T value);

// Prefix

T WaveMaskPrefixProduct<T>(WaveMask mask, T expr);

T WaveMaskPrefixSum<T>(WaveMask mask, T expr);

T WaveMaskPrefixMin<T>(WaveMask mask, T expr);

T WaveMaskPrefixMax<T>(WaveMask mask, T expr);

T WaveMaskPrefixBitAnd<T>(WaveMask mask, T expr);

T WaveMaskPrefixBitOr<T>(WaveMask mask, T expr);

T WaveMaskPrefixBitXor<T>(WaveMask mask, T expr);

uint WaveMaskPrefixCountBits(WaveMask mask, bool value);

// Communication

T WaveMaskReadLaneFirst<T>(WaveMask mask, T expr);

T WaveMaskBroadcastLaneAt<T>(WaveMask mask, T value, constexpr int lane);

T WaveMaskReadLaneAt<T>(WaveMask mask, T value, int lane);

T WaveMaskShuffle<T>(WaveMask mask, T value, int lane);
```
