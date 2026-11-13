// Regression test for https://github.com/shader-slang/slang/issues/13817 (diamond case).
//
// `top` calls `leaf` both directly and through `mid`, all over the associated type of a dynamic
// `IMaterial` with two conformances. Both calls use one set-specialization of `leaf`. The direct
// call is rewritten first, which lowers and memoizes that specialization; `mid`'s call must still
// pass its own runtime tag instead of picking up the memoized function and failing with E33180.
// `twoArg` is the same shape for a callee with a second parameter, which used to pair arguments
// with the wrong parameters and assert. The single-conformance runs check that the monomorphic
// case still compiles.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPV):-target spirv -entry computeMain -stage compute -conformance "A:IMaterial=0" -conformance "B:IMaterial=1"
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -conformance "A:IMaterial=0" -conformance "B:IMaterial=1"
//TEST:SIMPLE(filecheck=SPV):-target spirv -entry computeMain -stage compute -conformance "A:IMaterial=0"
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -conformance "A:IMaterial=0"

// SPV-NOT: error[
// SPV: OpEntryPoint
// HLSL-NOT: error[
// HLSL: computeMain

interface IBSDF { float eval(); }
struct BSDFA : IBSDF { float a; float eval() { return a; } }
struct BSDFB : IBSDF { float b; float c; float eval() { return b * c; } }

interface IMaterial { associatedtype BSDF : IBSDF; BSDF get_bsdf(); }
struct A : IMaterial { typealias BSDF = BSDFA; float4 data; BSDFA get_bsdf() { return { data.x }; } }
struct B : IMaterial { typealias BSDF = BSDFB; float4 data; BSDFB get_bsdf() { return { data.x, data.y }; } }

//TEST_INPUT: type_conformance A:IMaterial = 0
//TEST_INPUT: type_conformance B:IMaterial = 1

float leaf<T : IBSDF>(const T bsdf) { return bsdf.eval(); }
float mid<T : IBSDF>(const T bsdf) { return leaf(bsdf) * 2; }
float top<T : IBSDF>(const T bsdf) { return mid(bsdf) + leaf(bsdf); }

float leaf2<T : IBSDF>(const T bsdf, float x) { return bsdf.eval() + x; }
float mid2<T : IBSDF>(const T bsdf) { return leaf2(bsdf, 1.0) * 2; }
float twoArg<T : IBSDF>(const T bsdf) { return leaf2(bsdf, 3.0) + mid2(bsdf); }

//TEST_INPUT:ubuffer(data=[1.0 2.0 0.0 0.0  3.0 4.0 0.0 0.0  5.0 6.0 0.0 0.0  7.0 8.0 0.0 0.0], stride=16):name=materials
StructuredBuffer<float4> materials;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    const IMaterial material = createDynamicObject<IMaterial>(tid.x & 1, materials[tid.x]);
    let bsdf = material.get_bsdf();
    // top(b) = 3 * e and twoArg(b) = 3 * e + 5, where e = b.eval() is data.x on A lanes and
    // data.x * data.y on B lanes.
    outputBuffer[tid.x] = top(bsdf);
    outputBuffer[tid.x + 4] = twoArg(bsdf);
}

// CHECK: 3
// CHECK-NEXT: 36
// CHECK-NEXT: 15
// CHECK-NEXT: 168
// CHECK-NEXT: 8
// CHECK-NEXT: 41
// CHECK-NEXT: 20
// CHECK-NEXT: 173
