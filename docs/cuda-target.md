Slang CUDA Target Support
=========================

Slang has preliminary support for producing CUDA source, and PTX binaries using nvrtc. 

# Features

* Can compile Slang source into CUDA source code
* Supports compute style shaders 
* Supports a 'bindless' CPU like model
* Can compile CUDA source to PTX through 'pass through' mechansism

# Limitations

These limitations apply to Slang transpiling to CUDA. 

* Only supports the 'texture object' style binding (The texture object API is only supported on devices of compute capability 3.0 or higher. )
* Samplers are not separate objects in CUDA - they are combined into a single 'TextureObject'. So samplers are effectively ignored on CUDA targets. 
* When using a TextureArray.Sample (layered texture in CUDA) - the index will be treated as an int, as this is all CUDA allows
* Care must be used in using `WaveGetLaneIndex` wave intrinsic - it will only give the right results for appropriate launches
* CUDA 'surfaces' are used for textures which are read/write. CUDA does NOT do format conversion with surfaces.

The following are a work in progress or not implemented but are planned to be so in the future

* Some resource types remain unsupported, and not all methods on types are supported

# How it works

For producing PTX binaries Slang uses nvrtc. Nvrtc dll/shared library has to be available to Slang (in the appropriate PATH for example) for it to be able to produce PTX. The nvrtc compiler can be accessed directly through

```
SLANG_PASS_THROUGH_NVRTC,  
```

Much like other targets that use downstream compilers Slang can be used to compile CUDA source directly to PTX via the pass through mechansism. That the Slang command line options will broadly be mapped down to the appropriate options for the nvrtc compilation. In the API the `SlangCompileTarget` for CUDA is `SLANG_CUDA_SOURCE` and for PTX is `SLANG_PTX`. These can also be specified on the Slang command line as `-target cuda` and `-target ptx`. 

Binding 
=======

Say we have some Slang source like the following:

```
struct Thing { int a; int b; }

Texture2D<float> tex;
SamplerState sampler;
RWStructuredBuffer<int> outputBuffer;        
ConstantBuffer<Thing> thing3;        
        
[numthreads(4, 1, 1)]
void computeMain(
    uint3 dispatchThreadID : SV_DispatchThreadID, 
    uniform Thing thing, 
    uniform Thing thing2)
{
   // ...
}
```

This will be turned into a CUDA entry point with 

```
struct UniformEntryPointParams
{
    Thing thing;
    Thing thing2;
};

struct UniformState
{
    CUtexObject tex;                // This is the combination of a texture and a sampler(!)
    SamplerState sampler;           // This variable exists within the layout, but it's value is not used.
    RWStructuredBuffer<int32_t> outputBuffer;    // This is implemented as a template in the CUDA prelude. It's just a pointer, and a size
    Thing* thing3;                  // Constant buffers map to pointers
};   

// [numthreads(4, 1, 1)]
extern "C" __global__  void computeMain(UniformEntryPointParams* params, UniformState* uniformState)
```

With CUDA - the caller specifies how threading is broken up, so `[numthreads]` is available through reflection, and in a comment in output source code but does not produce varying code. 

The UniformState and UniformEntryPointParams struct typically vary by shader. UniformState holds 'normal' bindings, whereas UniformEntryPointParams hold the uniform entry point parameters. Where specific bindings or parameters are located can be determined by reflection. The structures for the example above would be something like the following... 

`StructuredBuffer<T>`,`RWStructuredBuffer<T>` become

```
    T* data;
    size_t count;
```    

`ByteAddressBuffer`, `RWByteAddressBuffer` become 

```
    uint32_t* data;
    size_t sizeInBytes;
```  



## Texture

Read only textures will be bound as the opaque CUDA type CUtexObject. This type is the combination of both a texture AND a sampler. This is somewhat different from HLSL, where there can be separate `SamplerState` variables. This allows access of a single texture binding with different types of sampling. 

If code relys on this behavior it will be necessary to bind multiple CtexObjects with different sampler settings, accessing the same texture data. 

Slang has some preliminary support for TextureSampler type - a combined Texture and SamplerState. To write Slang code that can target CUDA and other platforms using this mechanism will expose the semantics appropriately within the source.  
 
Load is only supported for Texture1D, and the mip map selection argument is ignored. This is because there is tex1Dfetch and no higher dimensional equivalents. CUDA also only allows such access if the backing array is linear memory - meaning the bound texture cannot have mip maps - thus making the mip map parameter superflous anyway. RWTexture does allow Load on other texture types.  
 
## RWTexture 
 
RWTexture types are converted into CUsurfObject type. 

In CUDA it is not possible to do a format conversion on an access to a CUsurfObject, so it must be backed by the same data format as is used within the Slang source code. 

It is also worth noting that CUsurfObjects in CUDA are NOT allowed to have mip maps. 

By default surface access uses cudaBoundaryModeZero, this can be replaced using the macro SLANG_CUDA_BOUNDARY_MODE in the CUDA prelude.

## Sampler

Samplers are in effect ignored in CUDA output. Currently we do output a variable `SamplerState`, but this value is never accessed within the kernel and so can be ignored. More discussion on this behavior is in `Texture` section.

## Unsized arrays

Unsized arrays can be used, which are indicated by an array with no size as in `[]`. For example 

```
    RWStructuredBuffer<int> arrayOfArrays[];
```

With normal 'sized' arrays, the elements are just stored contiguously within wherever they are defined. With an unsized array they map to `Array<T>` which is...

```
    T* data;
    size_t count;
```    

Note that there is no method in the shader source to get the `count`, even though on the CUDA target it is stored and easily available. This is because of the behavior on GPU targets 

* That the count has to be stored elsewhere (unlike with CUDA) 
* On some GPU targets there is no bounds checking - accessing outside the bound values can cause *undefined behavior*
* The elements may be laid out *contiguously* on GPU

In practice this means if you want to access the `count` in shader code it will need to be passed by another mechanism - such as within a constant buffer. It is possible in the future support may be added to allow direct access of `count` work across targets transparently. 

## Prelude

For CUDA the code to support the code generated by Slang is partly defined within the 'prelude'. The prelude is inserted text placed before the generated CUDA source code. For the Slang command line tools as well as the test infrastructure, the prelude functionality is achieved through a `#include` in the prelude text of the `prelude/slang-cuda-prelude.h` specified with an absolute path. Doing so means other files the `slang-cuda-prelude.h` might need can be specified relatively, and include paths for the backend compiler do not need to be modified. 

The prelude needs to define 

* 'Built in' types (vector, matrix, 'object'-like Texture, SamplerState etc) 
* Scalar intrinsic function implementations
* Compiler based definations/tweaks 

For a client application - as long as the requirements of the generated code are met, the prelude can be implemented by whatever mechanism is appropriate for the client. For example the implementation could be replaced with another implementation, or the prelude could contain all of the required text for compilation. Setting the prelude text can be achieved with the method on the global session...

```
/** Set the 'prelude' for generated code for a 'downstream compiler'.
@param passThrough The downstream compiler for generated code that will have the prelude applied to it. 
@param preludeText The text added pre-pended verbatim before the generated source

That for pass-through usage, prelude is not pre-pended, preludes are for code generation only. 
*/

void setDownstreamCompilerPrelude(SlangPassThrough passThrough, const char* preludeText);
```

The code that sets up the prelude for the test infrastucture and command line usage can be found in ```TestToolUtil::setSessionDefaultPrelude```. Essentially this determines what the absolute path is to `slang-cpp-prelude.h` is and then just makes the prelude `#include "the absolute path"`.

Wave Intrinsics
===============

There is broad support for [HLSL Wave intrinsics](https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12), including support for [SM 6.5 intrinsics](https://microsoft.github.io/DirectX-Specs/d3d/HLSL_ShaderModel6_5.html). 

Most Wave intrinsics will work with vector, matrix or scalar types of typical built in types - uint, int, float, double, uint64_t, int64_t.  

The support is provided via both the Slang stdlib as well as the Slang CUDA prelude found in 'prelude/slang-cuda-prelude.h'. Many Wave intrinsics are not directly applicable within CUDA which supplies a more low level mechanisms. The implementation of most Wave functions work most optimally if a 'Wave' where all lanes are used. If all lanes from index 0 to pow2(n) -1  are used (which is also true if all lanes are used) a binary reduction is typically applied. If this is not the case the implementation fallsback on a slow path which is linear in the number of active lanes, and so is typically significantly less performant. 

For more a more concrete example take 

```
int sum = WaveActiveSum(...);
```

When computing the sum, if all lanes (32 on CUDA), the computation will require 5 steps to complete (2^5 = 32). If say just one lane is not being used it will take 31 steps to complete (because it is now linear in amount of lanes). So just having one lane disabled required 6 times as many steps. If lanes with 0 - 15 are active, it will take 4 steps to complete (2^4 = 16). 

In the future it may be possible to improve on the performance of the 'slow' path, however it will always remain the most efficient generally for all of 0 to pow2(n) - 1 lanes to be active. 

It is also worth noting that lane communicating intrinsics performance will be impacted by the 'size' of the data communicated. The size here is at a minimum the amount of built in scalar types used in the processing. The CUDA language only allows direct communication with built in scalar types. 

Thus 

```
int3 v = ...;
int3 sum = WaveActiveSum(v);
```

Will require 3 times as many steps as the earlier scalar example just using a single int. 

## WaveGetLaneIndex

'WaveGetLaneIndex' defaults to `(threadIdx.x & SLANG_CUDA_WARP_MASK)`. Depending on how the kernel is launched this could be incorrect. There other ways to get lane index, for example using inline assembly. This mechanism though is apparently slower than the simple method used here. There is support for using the asm mechnism in the CUDA prelude using the `SLANG_USE_ASM_LANE_ID` preprocessor define to enable the feature. 

There is potential to calculate the lane id using the [numthreads] markup in Slang/HLSL, but that also requires some assumptions of how that maps to a lane index. 

## Unsupported Intrinsics

* Intrinsics which only work in pixel shaders
  + QuadXXXX intrinsics
  
Limitations
===========

Some features are not available because they cannot be mapped with appropriate behavior to a target. Other features are unavailable because of resources to devote to more unusual features. 

* Not all Wave intrinsics are supported
* There is not complete support for all methods on 'objects' like textures etc. 
* Does not currently support combined 'TextureSampler'. A Texture behaves equivalently to a TextureSampler and Samplers are ignored.
* Half type is not currently supported
* GetDimensions is not available on any Texture type currently - as there doesn't appear to be a CUDA equivalent 

Language aspects
================

# Arrays passed by Value

Slang follows the HLSL convention that arrays are passed by value. This is in contrast with CUDA where arrays follow C++ conventions and are passed by reference. To make generated CUDA follow this convention an array is turned into a 'FixedArray' struct type. 

To get something more similar to CUDA/C++ operation the array can be marked in out or inout to make it passed by reference. 
