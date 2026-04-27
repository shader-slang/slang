//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Verify that a ConstantBuffer with ONLY single-level pointers does not
// trigger any pointer-to-ulong lowering. Single-level pointers must stay
// as typed device pointers in buffer-bound structs.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct MyParams
{
    int* data;
    float* weights;
    int count;
};

ConstantBuffer<MyParams> params;

RWStructuredBuffer<int> output;

// All pointer fields must remain as typed device pointers (no ulong).
// CHECK: struct MyParams
// CHECK: int device* data
// CHECK: float device* weights
// CHECK: int count
// CHECK-NOT: ulong
// CHECK: };

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (int(tid.x) < params.count)
        output[tid.x] = params.data[tid.x] + int(params.weights[tid.x]);
}
