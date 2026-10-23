// The GLSL EXT SER wrappers in glsl.meta.slang whose SPIR-V branch was keyed on
// spvShaderInvocationReorderEXT instead of on `spirv` must compile for SPIR-V without an explicit
// SER capability, like hitObjectTraceRayEXT and the wrappers that forward to HitObject. They used
// to fail with E41011 ("__target_switch has no compatible target") unless
// -capability spvShaderInvocationReorderEXT was passed.

//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly -capability spvShaderInvocationReorderEXT
//TEST:SIMPLE_EX(filecheck=POS): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly -capability spvRayTracingPositionFetchKHR -DPOSITION_FETCH
//TEST:SIMPLE_EX(filecheck=NONV): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=NONV): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly -capability spvRayTracingPositionFetchKHR -DPOSITION_FETCH

// CHECK-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// CHECK-DAG: OpCapability ShaderInvocationReorderEXT
// CHECK-DAG: OpHitObjectGetRayTMinEXT
// CHECK-DAG: OpHitObjectGetRayTMaxEXT
// CHECK-DAG: OpHitObjectGetRayFlagsEXT
// CHECK-DAG: OpHitObjectGetWorldRayOriginEXT
// CHECK-DAG: OpHitObjectGetWorldRayDirectionEXT
// CHECK-DAG: OpHitObjectGetAttributesEXT
// CHECK-DAG: OpHitObjectReorderExecuteShaderEXT
// CHECK-DAG: OpHitObjectTraceReorderExecuteEXT

// POS-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// POS-DAG: OpCapability ShaderInvocationReorderEXT
// POS-DAG: OpCapability RayTracingPositionFetchKHR
// POS-DAG: OpHitObjectGetIntersectionTriangleVertexPositionsEXT

// No run uses the NV extension anywhere in the module.
// NONV-NOT: ShaderInvocationReorderNV

layout(binding = 0) uniform accelerationStructureEXT topLevel;
layout(location = 0) rayPayloadEXT vec4 payload;

void main()
{
    hitObjectEXT ho;
    hitObjectTraceRayEXT(ho, topLevel, 0u, 0xFFu, 0u, 0u, 0u,
        vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0, 0);

    vec4 r = vec4(hitObjectGetWorldRayOriginEXT(ho) + hitObjectGetWorldRayDirectionEXT(ho),
                  hitObjectGetRayTMinEXT(ho) + hitObjectGetRayTMaxEXT(ho));
    r.w += float(hitObjectGetRayFlagsEXT(ho));

    vec2 barycentrics;
    hitObjectGetAttributesEXT(ho, barycentrics);
    r.xy += barycentrics;

#ifdef POSITION_FETCH
    vec3 positions[3];
    hitObjectGetIntersectionTriangleVertexPositionsEXT(ho, positions);
    r.xyz += positions[0];
#endif

    hitObjectReorderExecuteEXT(ho, 0);
    hitObjectReorderExecuteEXT(ho, 1u, 4u, 0);
    hitObjectTraceReorderExecuteEXT(ho, topLevel, 0u, 0xFFu, 0u, 0u, 0u,
        vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0, 0);
    hitObjectTraceReorderExecuteEXT(ho, topLevel, 0u, 0xFFu, 0u, 0u, 0u,
        vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0, 1u, 4u, 0);

    payload = r;
}
