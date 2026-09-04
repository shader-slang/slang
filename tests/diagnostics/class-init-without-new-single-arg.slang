//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain

// https://github.com/shader-slang/slang/issues/12485

class Counter
{
    int v;
    __init(int start) { v = start * 2; }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Single-arg construction of a class without `new`, in both call form and
    // cast-syntax form: each must report E30066, not abort. Each site emits the
    // primary message followed by its source-span note.
    Counter viaCall = Counter(4);
    //CHECK: class can only be initialized by `new`
    //CHECK: a class can only be initialized by a `new` clause

    Counter viaCast = (Counter)4;
    //CHECK: class can only be initialized by `new`
    //CHECK: a class can only be initialized by a `new` clause

    outputBuffer[0] = viaCall.v + viaCast.v;
}
