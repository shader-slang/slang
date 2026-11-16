// CUDA short-circuit `&&` must keep a buffer load in its right operand inside
// the emitted expression, where nvcc evaluates it only when the left operand
// does not short-circuit (the `i < n && a[i]` guarded-access idiom).

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint count = output[0];
    if (tid.x < count && itemMin[tid.x].x > 0.0)
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK: &&
    //CHECK-NOT: bool _S
}
