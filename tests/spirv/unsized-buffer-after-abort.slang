//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -validate-ir -validate-with-downstream-compiler -O0
//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -validate-ir -validate-with-downstream-compiler -O3

StructuredBuffer<uint> inputs[2];
RWStructuredBuffer<uint> output;

[noinline]
void accessBuffers(uint3 tid)
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

// Abort becomes a terminator during SPIR-V legalization, which still visits the helper. Dead-code
// elimination then removes the unreachable helper and its local array of resource pointers,
// which SPIR-V cannot represent. This keeps the test focused on the deleted conversion's lifetime.
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    abort("stop before call");
    accessBuffers(tid);
}

// CHECK: OpAbortKHR
// CHECK-NEXT: OpFunctionEnd
