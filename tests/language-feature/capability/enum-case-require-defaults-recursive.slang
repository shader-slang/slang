//DIAGNOSTIC_TEST:SIMPLE(diag=RECURSIVE):-target hlsl -entry main -stage compute

enum Mode
{
    [require(hlsl)] A = 0,
}

// Capability checking visits both defaults but must stop following the recursive
// default's back edge. Lowering should still diagnose its unbounded expansion.
int recurse(Mode mode = Mode.A, int value = recurse(Mode(1)))
//RECURSIVE: maximum type nesting level exceeded
{
    return value + int(mode);
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = recurse();
}
