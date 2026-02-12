//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -Xslang -Wno-41020

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// Test tuples of interface types (issue #9837):
// 1. Tuple of interfaces with dynamic dispatch
// 2. Interface implementation with tuple fields
// 3. Nested tuple containing interface with tuple field
//

interface IFoo
{
    float eval();
}

// Simple implementations
struct FooA : IFoo
{
    float eval() { return 1.0; }
}

struct FooB : IFoo
{
    float eval() { return 2.0; }
}

// Implementation with tuple field
struct FooWithTuple : IFoo
{
    Tuple<float, int> data;

    __init(float f, int i)
    {
        data = makeTuple(f, i);
    }

    float eval() { return data._0; }
}

// Helper functions for tuple access (required pattern for dynamic dispatch)

// Test 1 helper: Tuple of interfaces with dynamic dispatch
// Tuples of interfaces should pack each element with its own tag+AnyValue
float2 evalPair(Tuple<IFoo, IFoo> pair)
{
    return float2(pair._0.eval(), pair._1.eval());
}

// Test 2 helper: Nested tuple containing interface with tuple field
float evalMixed(Tuple<IFoo, int> mixed)
{
    return mixed._0.eval();
}

int getMixedInt(Tuple<IFoo, int> mixed)
{
    return mixed._1;
}

// Test 4 helper: Triple of interfaces
float evalTriple(Tuple<IFoo, IFoo, IFoo> triple)
{
    return triple._0.eval() + triple._1.eval() + triple._2.eval();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    // Test 1: Tuple of interfaces with dynamic dispatch (simple impls)
    {
        Tuple<IFoo, IFoo> pair = { FooA(), FooB() };
        float2 result = evalPair(pair);
        outputBuffer[0] = result.x; // CHECK: 1.0
        outputBuffer[1] = result.y; // CHECK: 2.0
    }

    // Test 2: Tuple<IFoo, int> with FooWithTuple (impl has tuple field)
    {
        Tuple<IFoo, int> mixed = { FooWithTuple(3.5, 42), 42 };
        outputBuffer[2] = evalMixed(mixed);       // CHECK: 3.5
        outputBuffer[3] = float(getMixedInt(mixed)); // CHECK: 42.0
    }

    // Test 3: Tuple<IFoo, IFoo> where both are FooWithTuple instances
    {
        FooWithTuple ft1 = FooWithTuple(10.0, 1);
        FooWithTuple ft2 = FooWithTuple(20.0, 2);
        Tuple<IFoo, IFoo> pair = { ft1, ft2 };
        float2 result = evalPair(pair);
        outputBuffer[4] = result.x; // CHECK: 10.0
        outputBuffer[5] = result.y; // CHECK: 20.0
    }

    // Test 4: Mixed tuple with different interface implementations
    {
        // FooA returns 1.0, FooWithTuple returns its data._0, FooB returns 2.0
        Tuple<IFoo, IFoo, IFoo> triple = { FooA(), FooWithTuple(5.0, 99), FooB() };
        // 1.0 + 5.0 + 2.0 = 8.0
        outputBuffer[6] = evalTriple(triple); // CHECK: 8.0
    }

    // Test 5: Nested access - interface with tuple field, stored in tuple with int
    {
        FooWithTuple ft = FooWithTuple(7.5, 100);
        Tuple<IFoo, int> nested = { ft, 200 };
        float val = evalMixed(nested);
        float result = val + float(getMixedInt(nested));
        // 7.5 + 200.0 = 207.5
        outputBuffer[7] = result; // CHECK: 207.5
    }
}
