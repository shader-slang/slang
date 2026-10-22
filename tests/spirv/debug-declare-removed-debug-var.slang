// Regression test for https://github.com/shader-slang/slang/issues/13841
// When a pass removes a debug variable, it also removes the `DebugVarStorage` decoration of the
// storage that held it. Here specialization makes `T` void, so void-type cleanup removes the
// debug variables of `x` and `v`; it would keep a debug variable that a decoration still names.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -g2 -O0 -validate-ir

RWStructuredBuffer<float> outputBuffer;

[noinline]
void touch<T>(inout T x)
{
}

[noinline]
float useLocal<T>()
{
    T v;
    touch<T>(v);
    return 1.0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = useLocal<void>();
}

// CHECK: OpEntryPoint
// CHECK-NOT: DebugLocalVariable
