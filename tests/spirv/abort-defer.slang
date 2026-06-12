//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -skip-spirv-validation
// Note: -skip-spirv-validation is required because the bundled spirv-val does not yet
// know about SPV_KHR_shader_abort. Remove this flag once the validator is updated.

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

// CHECK: OpStore
// CHECK: OpAbortKHR
