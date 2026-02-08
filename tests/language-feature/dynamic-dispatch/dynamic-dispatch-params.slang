//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type

// Test for GitHub issue #9838: Dynamic Dispatch with Dynamic Dispatch Parameters
// Interface methods that take other dynamically-typed interface parameters should work correctly.
// This tests nested dynamic dispatch where ICombiner.combine takes IFoo and IBar as existential params.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float eval();
}

interface IBar
{
    float weight();
}

interface ICombiner
{
    float combine(IFoo foo, IBar bar); // Both params are existential
}

struct FooA : IFoo
{
    float eval() { return 1.0; }
}

struct FooB : IFoo
{
    float eval() { return 2.0; }
}

struct BarA : IBar
{
    float weight() { return 0.5; }
}

struct BarB : IBar
{
    float weight() { return 0.25; }
}

struct MyCombiner : ICombiner
{
    float combine(IFoo foo, IBar bar)
    {
        // Nested dynamic dispatch: foo.eval() and bar.weight()
        return foo.eval() * bar.weight();
    }
}

struct AddCombiner : ICombiner
{
    float combine(IFoo foo, IBar bar)
    {
        // Different combiner implementation
        return foo.eval() + bar.weight();
    }
}

float testCombine(ICombiner c, IFoo f, IBar b)
{
    // Triple dynamic dispatch happens here
    return c.combine(f, b);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ICombiner mulCombiner = MyCombiner();
    ICombiner addCombiner = AddCombiner();
    IFoo fooA = FooA();
    IFoo fooB = FooB();
    IBar barA = BarA();
    IBar barB = BarB();

    // Test 1: MyCombiner with FooA(1.0) * BarA(0.5) = 0.5
    outputBuffer[0] = testCombine(mulCombiner, fooA, barA);
    // CHECK: 0.5

    // Test 2: MyCombiner with FooB(2.0) * BarB(0.25) = 0.5
    outputBuffer[1] = testCombine(mulCombiner, fooB, barB);
    // CHECK: 0.5

    // Test 3: AddCombiner with FooA(1.0) + BarA(0.5) = 1.5
    outputBuffer[2] = testCombine(addCombiner, fooA, barA);
    // CHECK: 1.5

    // Test 4: AddCombiner with FooB(2.0) + BarB(0.25) = 2.25
    outputBuffer[3] = testCombine(addCombiner, fooB, barB);
    // CHECK: 2.25
}
