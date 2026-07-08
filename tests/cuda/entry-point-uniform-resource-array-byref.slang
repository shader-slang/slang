//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// The by-reference conversion applies equally when the descriptor array's elements
// are true resource types (structured buffers) rather than pointer-backed structs,
// as long as the array is carried by a struct-typed parameter.

struct Args
{
    RWStructuredBuffer<float> tensors[8];
}

// CHECK: __global__ void computeMain(Args_0* args_0, StructuredBuffer<uint> indices_0)
// CHECK: args_0->tensors_0[

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uniform Args args, uniform StructuredBuffer<uint> indices)
{
    args.tensors[indices[tid.x]][tid.x] = 1.0f;
}
