//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK):-target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK):-target wgsl -stage compute -entry computeMain

// https://github.com/shader-slang/slang/issues/12485
// glsl/spirv are omitted: a class object in a shader fails to emit on those
// targets independently of this fix (the already-working `new C({x})` form
// fails the same way there), so they cannot exercise this case.

class Counter
{
    int v;
    __init(int start) { v = start * 2; }
    int get() { return v; }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Counter c = new Counter(4 + int(tid.x));
    outputBuffer[0] = c.get();
    //CHECK: result code = 0
}
