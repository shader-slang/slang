//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir

// Regression test for https://github.com/shader-slang/slang/issues/13841
// `arr` is a local array of an empty struct that keeps its var, because it is indexed
// dynamically. Type legalization removes its debug variable, so it also removes the
// `DebugVarStorage` decoration that names the debug variable; with the decoration left in place,
// legalization would fail on an operand it cannot legalize.

struct Empty
{
}

struct HasEmpty
{
    Empty e;
    float x;
}

RWStructuredBuffer<float> outputBuffer;

[noinline]
void touch(inout Empty e)
{
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Empty arr[2];
    touch(arr[tid.x & 1]);
    HasEmpty h;
    h.x = outputBuffer[0];
    touch(h.e);
    outputBuffer[0] = h.x;
}

// CHECK: OpEntryPoint
// CHECK-NOT: = OpString "arr"
