// HitObject::GetRayTCurrent() must be available and lower to the committed-hit ray TMax on the SER
// targets that split into NV and EXT extension variants: GLSL and SPIR-V. Both variants are checked
// per target because the emitter picks the hitObject type (and thus the builtin/opcode) from the
// requested capability, and the two must stay paired. HLSL/DXR lowering is covered by the DXR-native
// tests and CUDA/OptiX by hit-object-get-ray-tcurrent-cuda.slang, which have no such NV/EXT split.

//TEST:SIMPLE(filecheck=NV):      -target spirv-asm -entry rayGenShader -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=EXT):     -target spirv-asm -entry rayGenShader -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT
//TEST:SIMPLE(filecheck=GLSL_NV): -target glsl -entry rayGenShader -stage raygeneration -capability GL_NV_shader_invocation_reorder -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL_EXT):-target glsl -entry rayGenShader -stage raygeneration -capability GL_EXT_shader_invocation_reorder -line-directive-mode none

//TEST_INPUT: set sceneBVH = AccelerationStructure
uniform RaytracingAccelerationStructure sceneBVH;

//TEST_INPUT:set resultBuffer = out ubuffer(data=[0 0], stride=4)
uniform RWStructuredBuffer<float> resultBuffer;

struct MyPayload
{
    float val;
};

[shader("raygeneration")]
void rayGenShader()
{
    RayDesc ray;
    ray.Origin = float3(0.1, 0.1, 0.1);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    MyPayload payload = { 0.0 };

    HitObject hitobj = HitObject::TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    // GetRayTCurrent() returns the committed-hit ray TMax, so on each SER target it must lower to
    // that target's *TMax* accessor, and to that extension's variant only: pairing a hitObjectNV
    // with the EXT builtin (or vice versa) is a type error the emitter must never produce.
    //
    // The opposite-variant op is rejected on both sides of the expected op — a leading -NOT (after
    // the extension/capability declarations) catches one emitted before it, a trailing -NOT catches
    // one emitted after — so the opposite op cannot appear anywhere in the output.
    // NV-DAG: OpExtension "SPV_NV_shader_invocation_reorder"
    // NV-DAG: OpCapability ShaderInvocationReorderNV
    // NV-NOT: OpHitObjectGetRayTMaxEXT
    // NV: OpHitObjectGetRayTMaxNV
    // NV-NOT: OpHitObjectGetRayTMaxEXT

    // EXT-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
    // EXT-DAG: OpCapability ShaderInvocationReorderEXT
    // EXT-NOT: OpHitObjectGetRayTMaxNV
    // EXT: OpHitObjectGetRayTMaxEXT
    // EXT-NOT: OpHitObjectGetRayTMaxNV

    // GLSL_NV-NOT: hitObjectGetRayTMaxEXT
    // GLSL_NV: hitObjectGetRayTMaxNV
    // GLSL_NV-NOT: hitObjectGetRayTMaxEXT
    // GLSL_EXT-NOT: hitObjectGetRayTMaxNV
    // GLSL_EXT: hitObjectGetRayTMaxEXT
    // GLSL_EXT-NOT: hitObjectGetRayTMaxNV
    resultBuffer[0] = hitobj.GetRayTCurrent();
}
