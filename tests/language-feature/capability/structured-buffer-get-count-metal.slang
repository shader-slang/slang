// Capability inference must check a callee before it reads the callee's requirements.
// sbGetCount (Metal lowers structured buffers to bare device pointers without length metadata)
// and callsLaterSibling depend on that order; rwsbGetCount and sbGetDimensions are controls that
// do not. Buffer<T>.getCount() compiles on Metal, where it lowers to get_width(), and is rejected
// on C++, which has no implementation.
// non-exhaustive: the "see definition of" notes point into hlsl.meta.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=SB_COUNT,non-exhaustive): -target metal -entry sbGetCount -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RWSB_COUNT,non-exhaustive): -target metal -entry rwsbGetCount -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SB_DIMS,non-exhaustive): -target metal -entry sbGetDimensions -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=LATER_SIBLING,non-exhaustive): -target metal -entry callsLaterSibling -stage compute
//TEST:SIMPLE(filecheck=METAL_BUFFER): -target metal -entry bufferGetCount -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CPP_BUFFER,non-exhaustive): -target cpp -entry bufferGetCount -stage compute

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry sbGetCount -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry sbGetCount -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry sbGetCount -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry sbGetCount -stage compute
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry sbGetCount -stage compute
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry sbGetCount -stage compute

// HLSL: .GetDimensions(
// SPIRV: OpArrayLength
// GLSL: ._data.length()
// WGSL: arrayLength(
// CUDA: .GetDimensions(
// CPP: .GetDimensions(

// METAL_BUFFER: get_width()

StructuredBuffer<uint> input;
RWStructuredBuffer<uint> output;
Buffer<uint> typedInput;

[numthreads(64, 1, 1)]
void sbGetCount(uint3 tid: SV_DispatchThreadID)
/*SB_COUNT:
     ^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^ entrypoint 'sbGetCount' uses features that are not available in 'compute' stage for 'metal' compilation target.
*/
{
    if (tid.x < input.getCount())
//SB_COUNT:           ^^^^^^^^ see using of 'getCount'
        output[tid.x] = input[tid.x];
}

[numthreads(64, 1, 1)]
void rwsbGetCount(uint3 tid: SV_DispatchThreadID)
/*RWSB_COUNT:
     ^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^ entrypoint 'rwsbGetCount' uses features that are not available in 'compute' stage for 'metal' compilation target.
*/
{
    if (tid.x < output.getCount())
//RWSB_COUNT:          ^^^^^^^^ see using of 'getCount'
        output[tid.x] = tid.x;
}

[numthreads(64, 1, 1)]
void sbGetDimensions(uint3 tid: SV_DispatchThreadID)
/*SB_DIMS:
     ^^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^^ entrypoint 'sbGetDimensions' uses features that are not available in 'compute' stage for 'metal' compilation target.
*/
{
    uint count;
    uint stride;
    input.GetDimensions(count, stride);
//SB_DIMS:^^^^^^^^^^^^^ see using of 'GetDimensions'
    if (tid.x < count)
        output[tid.x] = input[tid.x];
}

// A method can call a sibling that the checker has not capability-checked yet. `Box` is generic
// because only a generic member reference reaches `second` through processDeclModifiers alone; a
// non-generic `Box` reaches it through processReferencedDecl first and does not test that order.
struct Box<T>
{
    uint first() { return this.second(); }

    [require(hlsl)]
    uint second() { return 1; }
}

[numthreads(1, 1, 1)]
void callsLaterSibling()
/*LATER_SIBLING:
     ^^^^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^^^^ entrypoint 'callsLaterSibling' uses features that are not available in 'compute' stage for 'metal' compilation target.
*/
{
    Box<uint> box;
    output[0] = box.first();
}

[numthreads(1, 1, 1)]
void bufferGetCount()
/*CPP_BUFFER:
     ^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^ entrypoint 'bufferGetCount' uses features that are not available in 'compute' stage for 'cpp' compilation target.
*/
{
    output[0] = typedInput.getCount();
//CPP_BUFFER:              ^^^^^^^^ see using of 'getCount'
}
