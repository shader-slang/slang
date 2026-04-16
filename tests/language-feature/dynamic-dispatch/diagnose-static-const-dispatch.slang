// This test was previously disabled while static const dispatch through
// interfaces caused ICE 99999 (see #10150).
//
// The fix was implemented in #10150 and the functionality is now tested by
// static-const-dispatch.slang in this directory.
//
// This file is kept as a marker for the former diagnostic test.
// The DISABLE_TEST directives are left in place since the scenarios they
// targeted are covered by the new compute test.

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
