//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -O3

// Verify that defer lowering inserts cleanup before abort(), even though Abort is
// represented as a normal intrinsic until SPIR-V legalization turns it into a
// block terminator.

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    if (dispatchThreadID.x > 0)
    {
        defer outputBuffer[0] = 1;
        abort("defer cleanup");
    }
    outputBuffer[1] = 2;
}

// CHECK: [[DEFER_ADDR:%[A-Za-z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_uint %outputBuffer %int_0 %int_0
// CHECK-NEXT: OpStore [[DEFER_ADDR]] %uint_1
// CHECK-NEXT: OpAbortKHR
