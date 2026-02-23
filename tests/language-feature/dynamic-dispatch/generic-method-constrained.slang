// Test: Generic methods with interface type constraints (method<T : IFoo>()) called through
// dynamic dispatch. Verifies that when T is constrained to a user-defined interface,
// specializations dispatch correctly at runtime.
// See also: generic-method-dispatch.slang (unconstrained generics), issue #9886

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// User-defined interface used as a type constraint
interface IValue
{
    float get();
}

struct Constant : IValue
{
    float val;
    float get() { return val; }
}

struct Doubled : IValue
{
    float val;
    float get() { return val * 2.0; }
}

// Interface whose methods have generic type parameters constrained by IValue
interface IProcessor
{
    // Generic method: T must conform to IValue
    float compute<T : IValue>(T v);
    float computeScaled<T : IValue>(T v, float scale);
}

struct AddOneProcessor : IProcessor
{
    float compute<T : IValue>(T v) { return v.get() + 1.0; }
    float computeScaled<T : IValue>(T v, float scale) { return v.get() * scale + 1.0; }
}

struct NegateProcessor : IProcessor
{
    float compute<T : IValue>(T v) { return -(v.get()); }
    float computeScaled<T : IValue>(T v, float scale) { return -(v.get() * scale); }
}

IProcessor makeProcessor(int id)
{
    if (id == 0) return AddOneProcessor();
    else return NegateProcessor();
}

// Test: compute<Constant> dispatched through IProcessor
float testComputeConstant(int id, float val)
{
    IProcessor p = makeProcessor(id);
    Constant c = { val };
    return p.compute<Constant>(c);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dynamic dispatch code{{.*}} 2 possible types:
}

// Test: compute<Doubled> — same method, different conforming type argument
float testComputeDoubled(int id, float val)
{
    IProcessor p = makeProcessor(id);
    Doubled d = { val };
    return p.compute<Doubled>(d);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dynamic dispatch code{{.*}} 2 possible types:
}

// Test: two-argument generic method with interface constraint
float testComputeScaled(int id, float val, float scale)
{
    IProcessor p = makeProcessor(id);
    Constant c = { val };
    return p.computeScaled<Constant>(c, scale);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dynamic dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // id is a runtime value (== 0 for the single thread) so the compiler cannot
    // propagate constants into the helper functions and optimize away the dispatch.

    // compute<Constant>: AddOneProcessor returns val+1, NegateProcessor returns -val
    outputBuffer[0] = testComputeConstant(id,     4.0);  // CHECK: 5
    outputBuffer[1] = testComputeConstant(id + 1, 4.0);  // CHECK: -4

    // compute<Doubled>: same method, different T — Doubled.get() = val*2
    // AddOneProcessor: 3.0*2 + 1 = 7, NegateProcessor: -(3.0*2) = -6
    outputBuffer[2] = testComputeDoubled(id,     3.0);   // CHECK: 7
    outputBuffer[3] = testComputeDoubled(id + 1, 3.0);   // CHECK: -6

    // computeScaled<Constant>: AddOneProcessor returns val*scale+1, NegateProcessor returns -(val*scale)
    outputBuffer[4] = testComputeScaled(id,     2.0, 3.0); // CHECK: 7
    outputBuffer[5] = testComputeScaled(id + 1, 2.0, 3.0); // CHECK: -6
}
