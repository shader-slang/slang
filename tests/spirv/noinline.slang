//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -emit-spirv-directly

// Test that [noinline] attribute results in DontInline function control in SPIRV

RWStructuredBuffer<int> outputBuffer;

// CHECK: %helperFunc = OpFunction {{.*}} DontInline
[noinline]
int helperFunc(int x)
{
    return x + 1;
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = helperFunc(42);
}
