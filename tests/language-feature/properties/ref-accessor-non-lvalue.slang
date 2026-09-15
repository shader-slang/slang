// ref-accessor-non-lvalue.slang
//
// A `ref` accessor must return an l-value (its lowered IR helper returns a
// pointer to storage). A body that returns a non-l-value expression is
// diagnosed rather than mis-lowered or aborting the compiler.
// https://github.com/shader-slang/slang/issues/12487

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

struct Cell
{
    int _v;
    property int value
    {
        [mutating] ref { return _v + 1; }
//CHECK:                           ^ the form of this l-value argument is not valid for a `ref` parameter
    }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Cell c;
    c._v = 0;
    c.value = 3;
    outputBuffer[0] = c.value;
}
