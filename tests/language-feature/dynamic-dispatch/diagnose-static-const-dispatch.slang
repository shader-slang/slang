// Static const members in interfaces cannot currently be dispatched
// dynamically. Accessing a static const through an interface-typed value
// or a generic parameter with dynamic dispatch causes an internal compiler
// error (ICE 99999) instead of producing a proper diagnostic.
//
// These tests are disabled until the compiler either supports dynamic
// dispatch of static const members or produces a clear error message.

// Scenario 1: static const accessed on interface-typed value
//DISABLE_TEST:SIMPLE(filecheck=CHK1):-target hlsl -stage compute -entry scenario1

interface ILabeled
{
    static const int label;
    float compute(float x);
}

struct LinOp : ILabeled
{
    float scale;
    static const int label = 10;
    float compute(float x) { return x * scale; }
}

struct QuadOp : ILabeled
{
    float scale;
    static const int label = 20;
    float compute(float x) { return x * x * scale; }
}

int getLabel(ILabeled op)
{
    // CHK1: error
    return op.label;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void scenario1()
{
    ILabeled op;
    if (outputBuffer[0] == 0)
        op = LinOp(2.0);
    else
        op = QuadOp(3.0);
    outputBuffer[0] = getLabel(op);
}

// Scenario 2: static const accessed via generic T.label with dynamic dispatch
//DISABLE_TEST:SIMPLE(filecheck=CHK2):-target hlsl -stage compute -entry scenario2

int getLabelGeneric<T : ILabeled>(T op)
{
    // CHK2: error
    return T.label;
}

[numthreads(1, 1, 1)]
void scenario2()
{
    ILabeled op;
    if (outputBuffer[0] == 0)
        op = LinOp(2.0);
    else
        op = QuadOp(3.0);
    outputBuffer[0] = getLabelGeneric(op);
}
