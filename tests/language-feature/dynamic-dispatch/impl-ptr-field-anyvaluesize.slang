// Pointer fields contribute their platform size (typically 8 bytes on 64-bit)
// to anyValueSize. This test verifies that auto-inferred anyValueSize works
// correctly when implementations contain pointer fields, by comparing behavior
// with and without explicit [anyValueSize] annotation.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 1 0 1], stride=4):name=typeSelector
RWStructuredBuffer<uint> typeSelector;

// Auto-inferred anyValueSize: the compiler determines the size from the
// largest conforming type's layout including pointer fields.
interface IAutoSized
{
    int get();
}

struct AutoWithPtr : IAutoSized
{
    int* data;
    int offset;
    int get() { return *data + offset; }
}

struct AutoNoPtr : IAutoSized
{
    int val;
    int get() { return val; }
}

// Explicitly annotated anyValueSize: must be large enough to hold a pointer
// (8 bytes) + an int (4 bytes), with alignment padding.
[anyValueSize(16)]
interface IExplicitSized
{
    int get();
}

struct ExplicitWithPtr : IExplicitSized
{
    int* data;
    int offset;
    int get() { return *data + offset; }
}

struct ExplicitNoPtr : IExplicitSized
{
    int val;
    int get() { return val; }
}

IAutoSized makeAutoSized(int id, int* pData)
{
    if (id == 0)
    {
        AutoWithPtr a;
        a.data = pData;
        a.offset = 3;
        return a;
    }
    else
    {
        AutoNoPtr b;
        b.val = 77;
        return b;
    }
}

IExplicitSized makeExplicitSized(int id, int* pData)
{
    if (id == 0)
    {
        ExplicitWithPtr a;
        a.data = pData;
        a.offset = 3;
        return a;
    }
    else
    {
        ExplicitNoPtr b;
        b.val = 77;
        return b;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int source = 39;

    // Auto-sized with pointer: 39 + 3 = 42
    var a = makeAutoSized(typeSelector[0], &source);
    // CHECK: 42
    outputBuffer[0] = a.get();

    // Auto-sized without pointer: 77
    var b = makeAutoSized(typeSelector[1], nullptr);
    // CHECK: 77
    outputBuffer[1] = b.get();

    // Explicit-sized with pointer: 39 + 3 = 42
    var c = makeExplicitSized(typeSelector[2], &source);
    // CHECK: 42
    outputBuffer[2] = c.get();

    // Explicit-sized without pointer: 77
    var d = makeExplicitSized(typeSelector[3], nullptr);
    // CHECK: 77
    outputBuffer[3] = d.get();
}
