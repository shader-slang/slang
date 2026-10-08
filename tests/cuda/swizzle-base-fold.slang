// Regression test for https://github.com/shader-slang/slang/issues/12073
//
// A multi-component swizzle read (`.rgb`, `.xyz`, ...) has no native vector
// construction form in C++/CUDA, so the C-family emitter builds the result
// element by element, e.g. `float3{ base.x, base.y, base.z }`. If `base` is a
// value that would otherwise be folded into its use site (a texture fetch or
// buffer load), that fold used to textually duplicate the fetch once per
// component -- three loads for a `.rgb` -- producing three times the memory
// traffic in a hot loop, while SPIR-V (`OpVectorShuffle`) and HLSL (native
// `.xyz`) evaluate the base only once.
//
// `CPPSourceEmitter::shouldFoldInstIntoUseSites` now refuses to fold a base
// into a multi-element swizzle user, so the base materializes as a single
// temporary and the fetch/load is emitted exactly once. These checks assert
// that the load appears a single time in the emitted CUDA and C++ hot loop.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute

RWStructuredBuffer<float4> src;
RWStructuredBuffer<float3> dst;

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    float3 acc = float3(0.0);
    for (int i = 0; i < 8; ++i)
    {
        float3 s = src[tid + uint(i)].rgb;
        acc += s;
    }
    dst[tid] = acc;
}

// The `.rgb` read of `src[...]` (a buffer load) must be emitted exactly once:
// the load is bound to a single temporary, and the swizzle reads its three
// components from that same temporary rather than re-emitting the subscript
// once per component. Before the fix the subscript appeared three times inside
// the `float3{ ... }` / `Vector<float,3>{ ... }` initializer.
//
// Each check first captures the temporary the load is assigned to, then asserts
// the swizzle reads `.x`, `.y`, `.z` from that exact temporary. If the base were
// duplicated per component, the initializer would inline the subscript instead
// of the captured temporary and the second check would not match.

// CUDA: [[TMP:_S[0-9]+]] = *(&(globalParams_0->src_0)[
// CUDA: float3 {[[TMP]].x, [[TMP]].y, [[TMP]].z}

// CPP: [[CTMP:_S[0-9]+]] = *(&((&kernelContext_0)->globalParams_0->src_0)[
// CPP: Vector<float, 3> {[[CTMP]].x, [[CTMP]].y, [[CTMP]].z}
