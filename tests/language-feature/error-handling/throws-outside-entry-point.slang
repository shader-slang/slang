//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute
//
// Companion to tests/diagnostics/entry-point-cannot-throw.slang (issue #12330).
//
// The `entry-point-cannot-throw` diagnostic must reject `throws` only at the pipeline ABI
// boundary, never on ordinary functions. This test pins the shapes that must keep compiling:
// a plain throwing callee, a generic throwing callee, and an entry point that handles the
// error itself. It fails only in one direction, if the check ever widens from "the entry
// point declares `throws`" to "a `throws` clause is reachable from an entry point".

//CHECK: OpEntryPoint
//CHECK-NOT: error

RWStructuredBuffer<uint> outputBuffer;

uint mayFail(uint n) throws uint
{
    if (n < 1)
        throw 0xE1U;

    return n;
}

T mayFailGeneric<T : IArithmetic>(T v, uint n) throws uint
{
    if (n < 1)
        throw 0xE3U;

    return v;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    do
    {
        outputBuffer[tid.x] = try mayFail(tid.x) + (uint)(try mayFailGeneric<int>(7, tid.x));
    }
    catch (err : uint)
    {
        outputBuffer[tid.x] = err;
    }
}
