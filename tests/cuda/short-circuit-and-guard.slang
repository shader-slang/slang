// CUDA short-circuit `&&` must keep a buffer load in its right operand inside
// the emitted expression (the `i < n && a[i]` guarded-access idiom). Issue
// #14494's SASS measurements are what make that shape matter downstream —
// nvcc evaluates an in-expression operand only when the left one does not
// short-circuit; this test pins the emitted-source shape itself: the load
// text must appear inside the `if` line, not as a preceding statement.

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

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
    //CHECK-NOT: bool _S
    //CHECK: if({{.*}}&&{{.*}}slang_ldg_0
}
