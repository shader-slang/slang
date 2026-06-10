//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv -capability abort -skip-spirv-validation
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -capability abort
// Note: -skip-spirv-validation is required because the bundled spirv-val does not yet
// know about SPV_KHR_shader_abort. Remove this flag once the validator is updated.

// Verify that abort() emits OpAbortKHR for SPIR-V with the required capability and
// extension, packaging the format string + args into an explicitly laid out struct,
// and emits abortEXT() with the GL_EXT_shader_abort extension for GLSL.

// The format string is 26 chars + null terminator = 27 bytes, packed into 7 uint
// words = 28 bytes, so the uint argument lands at offset 28.

// CHECK_SPIRV: OpCapability AbortKHR
// CHECK_SPIRV: OpExtension "SPV_KHR_shader_abort"
// CHECK_SPIRV: OpDecorate {{.*}} ArrayStride 4
// CHECK_SPIRV: OpMemberDecorate {{.*}} 0 Offset 0
// CHECK_SPIRV: OpMemberDecorate {{.*}} 1 Offset 28
// CHECK_SPIRV: OpAbortKHR

// CHECK_GLSL: #extension GL_EXT_shader_abort
// CHECK_GLSL: abortEXT("thread id out of range: %u", {{.*}})

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    if (dispatchThreadID.x > 100)
    {
        abort("thread id out of range: %u", dispatchThreadID.x);
    }
}
