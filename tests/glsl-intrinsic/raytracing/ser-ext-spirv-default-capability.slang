// The GLSL EXT SER (hitObject*EXT) wrappers compile for -target spirv under the default profile,
// with no SER capability given, and emit the EXT extension and capability, never the NV ones.
//   (check)  every wrapper except the position-fetch and motion ones, with no capability, then
//            with the EXT SER capability given.
//   (pos)    hitObjectGetIntersectionTriangleVertexPositionsEXT, given only position fetch.
//   (motion) hitObjectTraceMotionReorderExecuteEXT, given only motion blur.
//   (nonv)   the check, pos and motion runs again, checking that neither the NV capability nor
//            the NV extension appears; a separate prefix, as a NOT placed after a DAG group only
//            scans the text after the group's last match.

//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly -capability spvShaderInvocationReorderEXT
//TEST:SIMPLE_EX(filecheck=POS): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly -capability spvRayTracingPositionFetchKHR -DPOSITION_FETCH
//TEST:SIMPLE_EX(filecheck=NONV): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=NONV): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly -capability spvRayTracingPositionFetchKHR -DPOSITION_FETCH
//TEST:SIMPLE_EX(filecheck=NONV): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly -capability spvRayTracingMotionBlurNV -DMOTION
//TEST:SIMPLE_EX(filecheck=MOTION): -lang glsl tests/glsl-intrinsic/raytracing/ser-ext-spirv-default-capability.slang -stage raygeneration -entry main -target spirv-asm -emit-spirv-directly -capability spvRayTracingMotionBlurNV -DMOTION

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

// MOTION-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// MOTION-DAG: OpCapability ShaderInvocationReorderEXT
// MOTION-DAG: OpCapability RayTracingMotionBlurNV
// MOTION-DAG: OpHitObjectTraceMotionReorderExecuteEXT

// NONV-NOT: ShaderInvocationReorderNV
// NONV-NOT: SPV_NV_shader_invocation_reorder

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

#ifdef MOTION
    hitObjectTraceMotionReorderExecuteEXT(ho, topLevel, 0u, 0xFFu, 0u, 0u, 0u,
        vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0, 0.5, 1u, 4u, 0);
#endif

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
