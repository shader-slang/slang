//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv -capability abort -skip-spirv-validation
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -capability abort
// Note: -skip-spirv-validation is required because the bundled spirv-val does not yet
// know about SPV_KHR_shader_abort. Remove this flag once the validator is updated.

// Verify that abort() emits OpAbortKHR for SPIR-V with the required capability and
// extension, packaging the format string + args into an explicitly laid out struct,
// and emits abortEXT() with the GL_EXT_shader_abort extension for GLSL.
//
// Covered payload shapes:
// - single uint arg: format is 26 chars + null = 27 bytes -> 7 uint words = 28
//   bytes, so the arg lands at offset 28
// - empty format string: still packs the null terminator -> 1-word array
// - no varargs: single-field struct
// - mixed-type varargs (uint, int, float, bool): format is 19 chars + null ->
//   5 words = 20 bytes; member offsets 20/24/28/32; bool widens to uint since
//   OpTypeBool has no physical layout

// CHECK_SPIRV: OpCapability AbortKHR
// CHECK_SPIRV: OpExtension "SPV_KHR_shader_abort"

// CHECK_SPIRV: OpDecorate {{.*}} ArrayStride 4
// CHECK_SPIRV: OpMemberDecorate %AbortMessage 0 Offset 0
// CHECK_SPIRV: OpMemberDecorate %AbortMessage 1 Offset 28
// CHECK_SPIRV: OpMemberDecorate %AbortMessage_0 0 Offset 0
// CHECK_SPIRV: OpMemberDecorate %AbortMessage_1 0 Offset 0
// CHECK_SPIRV: OpMemberDecorate %AbortMessage_2 0 Offset 0
// CHECK_SPIRV: OpMemberDecorate %AbortMessage_2 1 Offset 20
// CHECK_SPIRV: OpMemberDecorate %AbortMessage_2 2 Offset 24
// CHECK_SPIRV: OpMemberDecorate %AbortMessage_2 3 Offset 28
// CHECK_SPIRV: OpMemberDecorate %AbortMessage_2 4 Offset 32

// CHECK_SPIRV: %AbortMessage = OpTypeStruct %_arr_uint_int_7 %uint
// CHECK_SPIRV: %AbortMessage_0 = OpTypeStruct %_arr_uint_int_1
// CHECK_SPIRV: %AbortMessage_1 = OpTypeStruct %_arr_uint_int_4
// CHECK_SPIRV: %AbortMessage_2 = OpTypeStruct %_arr_uint_int_5 %uint %int %float %uint

// CHECK_SPIRV: OpAbortKHR %AbortMessage %
// CHECK_SPIRV: OpAbortKHR %AbortMessage_0 %
// CHECK_SPIRV: OpAbortKHR %AbortMessage_1 %
// CHECK_SPIRV: OpAbortKHR %AbortMessage_2 %

// CHECK_GLSL: #extension GL_EXT_shader_abort
// CHECK_GLSL: abortEXT("thread id out of range: %u", {{.*}})
// CHECK_GLSL: abortEXT("")
// CHECK_GLSL: abortEXT("just a message")
// CHECK_GLSL: abortEXT("u=%u i=%d f=%f b=%d", {{.*}})

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    if (dispatchThreadID.x > 100)
    {
        abort("thread id out of range: %u", dispatchThreadID.x);
    }
    if (dispatchThreadID.y > 100)
    {
        abort("");
    }
    if (dispatchThreadID.z > 100)
    {
        abort("just a message");
    }
    if (dispatchThreadID.x == 99)
    {
        abort("u=%u i=%d f=%f b=%d", dispatchThreadID.x, -1, 1.5f, dispatchThreadID.y > 5);
    }
}
