// A vector reshape/cast in the chain keeps its diamond: the C++/CUDA
// emitters refuse to fold a vector-typed reshape/cast instruction (its
// emitted text refers to the operand multiple times), so hoisting it would
// place the guarded load inside an unconditionally executed statement.
// (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 1 && any(int3(itemMin[tid.x]) > 2))
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK-NOT: &&
}
