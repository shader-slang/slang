//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -validate-ir -O0
//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -validate-ir -O3

StructuredBuffer<uint> inputs[2];
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Legalizing this dynamic array access queues its users, including the conversion in the
    // later block. Abort legalization then deletes that conversion before deferred cleanup.
    StructuredBuffer<uint> localInputs[2] = { inputs[0], inputs[1] };
    let src = localInputs[tid.x & 1];
    if (tid.y == 0)
    {
        abort("stop");
        __UnsizedStructuredBuffer<uint> view = src;
        output[0] = view[0];
    }
    else
        output[0] = src[0];
}

// CHECK: OpAbortKHR
// CHECK-NEXT: %{{[a-zA-Z0-9_]+}} = OpLabel
// CHECK: OpStore
