// When a method returning `This` is called on an interface-typed value,
// the result is packed as the interface type (existential), erasing the
// `This` identity. Passing that result to a method expecting `This` as
// a parameter is rejected because the compiler cannot prove the
// existential value matches the concrete `This` type.
//
// The current diagnostic (error 30019) is confusing: it prints both the
// expected and actual types as `This`, making it look like they match.
// A dedicated diagnostic should replace this with a clearer message
// explaining that the `This` type is not resolvable after type erasure.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain

interface ISelfOp
{
    This clone();
    bool matches(This other);
    float getValue();
}

struct ImplA : ISelfOp
{
    float v;
    This clone() { return ImplA(v); }
    bool matches(ImplA other) { return v == other.v; }
    float getValue() { return v; }
}

struct ImplB : ISelfOp
{
    float v;
    This clone() { return ImplB(v); }
    bool matches(ImplB other) { return v == other.v; }
    float getValue() { return v; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    ISelfOp obj;
    if (outputBuffer[0] == 0.0)
        obj = ImplA(5.0);
    else
        obj = ImplB(5.0);

    ISelfOp copy = obj.clone();
    // CHECK: error 30019
    outputBuffer[0] = obj.matches(copy) ? 1.0 : 0.0;
}
