// A [mutating] method on an implementation with a pointer field modifies the
// pointed-to value. After the mutating dispatch wrapper unpacks AnyValue to
// the concrete type, calls the method, and repacks, the pointer itself is
// preserved (same address) and the pointed-to memory reflects the mutation.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=typeSelector
RWStructuredBuffer<uint> typeSelector;

interface IAccumulator
{
    int get();
    [mutating] void accumulate(int amount);
}

// Accumulates into a pointed-to value
struct PtrAccumulator : IAccumulator
{
    int* target;

    int get()
    {
        return *target;
    }

    [mutating] void accumulate(int amount)
    {
        *target += amount;
    }
}

// Accumulates into a local field (no pointer)
struct LocalAccumulator : IAccumulator
{
    int value;

    int get()
    {
        return value;
    }

    [mutating] void accumulate(int amount)
    {
        value += amount;
    }
}

IAccumulator makeAccumulator(int id, int* target, int initial)
{
    if (id == 0)
    {
        PtrAccumulator a;
        a.target = target;
        return a;
    }
    else
    {
        LocalAccumulator a;
        a.value = initial;
        return a;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int storage = 0;

    // PtrAccumulator: mutations go through the pointer to `storage`
    var acc = makeAccumulator(typeSelector[0], __getAddress(storage), 0);
    acc.accumulate(10);
    acc.accumulate(20);
    // CHECK: 30
    outputBuffer[0] = acc.get();

    // Verify the mutation is visible through the original variable
    // CHECK: 30
    outputBuffer[1] = storage;

    // LocalAccumulator: mutations are local to the existential value
    var acc2 = makeAccumulator(typeSelector[1], nullptr, 5);
    acc2.accumulate(7);
    // CHECK: 12
    outputBuffer[2] = acc2.get();

    // Further mutation on PtrAccumulator
    acc.accumulate(12);
    // CHECK: 42
    outputBuffer[3] = acc.get();
    // CHECK: 42
    outputBuffer[4] = storage;
}
