// A multi-component swizzle of a vector-typed memory read does not fold into
// its use site (the C++/CUDA emitters rebuild it element by element, which
// would repeat the read), so a chain through such a swizzle must keep its
// diamond: hoisting would evaluate the read unconditionally. (Companion to
// shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 1 && any(itemMin[tid.x].xy > 1.0))
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK-NOT: &&
}
