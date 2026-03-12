// RWStructuredBuffer<IFoo> with multiple elements where each element
// is a different concrete type. Tests per-element dispatch through a
// loop, verifying that the buffer correctly tracks the concrete type
// of each element independently.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):name=typeSelector
RWStructuredBuffer<uint> typeSelector;

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
    float getValue() { return v + 100.0; }
}

struct ImplC : IFoo
{
    float v;
    float getValue() { return v * 10.0; }
}

//TEST_INPUT: type_conformance ImplA:IFoo = 0
//TEST_INPUT: type_conformance ImplB:IFoo = 1
//TEST_INPUT: type_conformance ImplC:IFoo = 2

// 4 elements, each 24 bytes = 6 uint32s, total 24 uint32s
//TEST_INPUT: set rwBuffer = ubuffer(data=[0 0 0 0 0 0  0 0 0 0 0 0  0 0 0 0 0 0  0 0 0 0 0 0], stride=4)
RWStructuredBuffer<IFoo> rwBuffer;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

IFoo makeObject(uint typeId, float val)
{
    switch (typeId % 3)
    {
    case 0:
        ImplA a;
        a.v = val;
        return a;
    case 1:
        ImplB b;
        b.v = val;
        return b;
    default:
        ImplC c;
        c.v = val;
        return c;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Populate buffer with alternating types based on typeSelector
    for (int i = 0; i < 4; i++)
    {
        rwBuffer[i] = makeObject(typeSelector[i], float(i + 1));
    }

    // Read back and dispatch each element independently
    for (int i = 0; i < 4; i++)
    {
        outputBuffer[i] = rwBuffer[i].getValue();
    }

    // typeSelector = [0, 1, 2, 3]
    // Element 0: ImplA(v=1.0) → 1.0
    // Element 1: ImplB(v=2.0) → 2.0 + 100.0 = 102.0
    // Element 2: ImplC(v=3.0) → 3.0 * 10.0 = 30.0
    // Element 3: ImplA(v=4.0) → 4.0 (3 % 3 = 0 → ImplA)

    // CHECK: 1.0
    // CHECK: 102.0
    // CHECK: 30.0
    // CHECK: 4.0
}
