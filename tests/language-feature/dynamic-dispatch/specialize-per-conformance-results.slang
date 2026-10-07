//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -conformance A:IValue=3 -conformance B:IValue=7 -warnings-as-errors 55219
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry computeMain -conformance A:IValue=3 -conformance B:IValue=7 -warnings-as-errors 55219

interface IValue { uint get(); }
struct A : IValue { uint get() { return 2; } }
struct B : IValue { uint get() { return 5; } }
struct Result { uint value; float weight; }

uint readMarker(inout uint state) { return state++; }

[__specializePerConformance]
Result result<T : IValue>(T value, inout uint state)
{
    uint marker = readMarker(state);
    return {value.get() + marker, float(value.get()) * 0.5};
}

[__specializePerConformance]
bool positive<T : IValue>(T value) { return value.get() > 0; }

[__specializePerConformance]
float weight<T : IValue>(T value) { return float(value.get()) * 0.5; }

//TEST_INPUT: type_conformance A:IValue = 3
//TEST_INPUT: type_conformance B:IValue = 7
//TEST_INPUT:ubuffer(data=[3 7], stride=4):name=ids
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
StructuredBuffer<uint> ids;
RWStructuredBuffer<uint> outputBuffer;
[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let v = createDynamicObject<IValue>(ids[tid.x], 0u);
    uint state = 10;
    Result r = result(v, state);
    outputBuffer[tid.x * 3] = r.value;
    outputBuffer[tid.x * 3 + 1] = state;
    outputBuffer[tid.x * 3 + 2] = uint(r.weight == weight(v) && positive(v));
}
// CHECK: 12
// CHECK: 11
// CHECK: 1
// CHECK: 15
// CHECK: 11
// CHECK: 1
// HLSL: = readMarker
// HLSL: = readMarker
// SPIRV: OpEntryPoint GLCompute
