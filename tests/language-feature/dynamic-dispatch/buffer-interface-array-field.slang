// RWStructuredBuffer of a struct containing a fixed-size array of
// interface-typed values. Each array element may hold a different
// concrete type, requiring independent dispatch per element.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute -shaderobj -output-using-type

[anyValueSize(8)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2.0; }
}

//TEST_INPUT: type_conformance ImplA:IFoo = 1
//TEST_INPUT: type_conformance ImplB:IFoo = 2

struct CompoundObject
{
    float scale;
    IFoo elements[2];
}

ImplA createImplA(float v) { ImplA a; a.v = v; return a; }
ImplB createImplB(float v) { ImplB b; b.v = v; return b; }

//TEST_INPUT: ubuffer(data=[2 5 3], stride=4):name=inputParams
StructuredBuffer<uint> inputParams;

// CompoundObject layout:
//   float scale (4 bytes, padded to alignment)
//   IFoo elements[0] (24 bytes: RTTI + WitnessTableID + AnyValue)
//   IFoo elements[1] (24 bytes)
//TEST_INPUT: set rwBuffer = ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<CompoundObject> rwBuffer;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    CompoundObject obj;
    obj.scale = float(inputParams[0]);
    obj.elements[0] = createImplA(float(inputParams[1]));
    obj.elements[1] = createImplB(float(inputParams[2]));

    rwBuffer[0] = obj;

    CompoundObject loaded = rwBuffer[0];

    // scale * elements[0].getValue() = 2.0 * 5.0 = 10.0
    outputBuffer[0] = loaded.scale * loaded.elements[0].getValue();

    // scale * elements[1].getValue() = 2.0 * (3.0 * 2.0) = 2.0 * 6.0 = 12.0
    outputBuffer[1] = loaded.scale * loaded.elements[1].getValue();

    // Verify each element dispatches independently
    outputBuffer[2] = loaded.elements[0].getValue();  // ImplA: 5.0
    outputBuffer[3] = loaded.elements[1].getValue();  // ImplB: 3.0 * 2.0 = 6.0

    // CHECK: 10.0
    // CHECK: 12.0
    // CHECK: 5.0
    // CHECK: 6.0
}
