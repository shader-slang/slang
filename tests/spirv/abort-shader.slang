//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability shader_abort -skip-spirv-validation

// Verify that abortShader emits OpAbortKHR with the required capability and extension,
// and packages the format string + args into a struct with scalar layout.

// CHECK: OpCapability AbortKHR
// CHECK: OpExtension "SPV_KHR_shader_abort"
// CHECK: OpDecorate {{.*}} ArrayStride 4
// CHECK: OpMemberDecorate {{.*}} 0 Offset 0
// CHECK: OpMemberDecorate {{.*}} 1 Offset {{[0-9]+}}
// CHECK: OpAbortKHR

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    if (dispatchThreadID.x > 100)
    {
        abortShader("thread id out of range: %u", dispatchThreadID.x);
    }
}
