// Regression test for https://github.com/shader-slang/slang/issues/13817 (diamond case).
//
// `top` calls `leaf` both directly and through `mid`, all over the associated type of a dynamic
// `IMaterial` with two conformances. The direct call and the call inside the set-specialized `mid`
// share one set-specialization of `leaf`; both must dispatch on the runtime tag instead of failing
// with E33180.

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

//TEST_INPUT:ubuffer(data=[1.0 2.0 0.0 0.0  3.0 4.0 0.0 0.0  5.0 6.0 0.0 0.0  7.0 8.0 0.0 0.0], stride=16):name=materials
StructuredBuffer<float4> materials;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    const IMaterial material = createDynamicObject<IMaterial>(tid.x & 1, materials[tid.x]);
    // top(b) = 3 * b.eval(): A lanes give 3 * data.x, B lanes give 3 * data.x * data.y.
    outputBuffer[tid.x] = top(material.get_bsdf());
}

// CHECK: 3
// CHECK-NEXT: 36
// CHECK-NEXT: 15
// CHECK-NEXT: 168
