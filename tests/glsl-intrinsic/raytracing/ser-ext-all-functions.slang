// Test all GLSL EXT SER intrinsics (GL_EXT_shader_invocation_reorder)

//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -stage raygeneration -entry main -target glsl

import glsl;

layout(binding = 0) uniform accelerationStructureEXT topLevel;
layout(location = 0) rayPayloadEXT vec4 payload;

// CHECK_GLSL: GL_EXT_shader_invocation_reorder

void main()
{
    hitObjectEXT ho;

    // CHECK_GLSL: hitObjectTraceRayEXT
    hitObjectTraceRayEXT(ho, topLevel, 0u, 0xFFu, 0u, 0u, 0u,
        vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0, 0);

    // CHECK_GLSL: hitObjectTraceRayMotionEXT
    hitObjectTraceRayMotionEXT(ho, topLevel, 0u, 0xFFu, 0u, 0u, 0u,
        vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0, 0.5, 0);

    // CHECK_GLSL: hitObjectRecordMissEXT
    hitObjectRecordMissEXT(ho, 0u, 0u, vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0);

    // CHECK_GLSL: hitObjectRecordMissMotionEXT
    hitObjectRecordMissMotionEXT(ho, 0u, 0u, vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0, 0.5);

    // CHECK_GLSL: hitObjectRecordEmptyEXT
    hitObjectRecordEmptyEXT(ho);

    // CHECK_GLSL: hitObjectExecuteShaderEXT
    hitObjectExecuteShaderEXT(ho, 0);

    // CHECK_GLSL-DAG: hitObjectIsEmptyEXT
    // CHECK_GLSL-DAG: hitObjectIsMissEXT
    // CHECK_GLSL-DAG: hitObjectIsHitEXT
    bool isEmpty = hitObjectIsEmptyEXT(ho);
    bool isMiss = hitObjectIsMissEXT(ho);
    bool isHit = hitObjectIsHitEXT(ho);

    // CHECK_GLSL-DAG: hitObjectGetRayFlagsEXT
    // CHECK_GLSL-DAG: hitObjectGetRayTMinEXT
    // CHECK_GLSL-DAG: hitObjectGetRayTMaxEXT
    // CHECK_GLSL-DAG: hitObjectGetWorldRayOriginEXT
    // CHECK_GLSL-DAG: hitObjectGetWorldRayDirectionEXT
    // CHECK_GLSL-DAG: hitObjectGetObjectRayOriginEXT
    // CHECK_GLSL-DAG: hitObjectGetObjectRayDirectionEXT
    uint rayFlags = hitObjectGetRayFlagsEXT(ho);
    float tmin = hitObjectGetRayTMinEXT(ho);
    float tmax = hitObjectGetRayTMaxEXT(ho);
    vec3 worldOrigin = hitObjectGetWorldRayOriginEXT(ho);
    vec3 worldDir = hitObjectGetWorldRayDirectionEXT(ho);
    vec3 objOrigin = hitObjectGetObjectRayOriginEXT(ho);
    vec3 objDir = hitObjectGetObjectRayDirectionEXT(ho);

    // CHECK_GLSL-DAG: hitObjectGetObjectToWorldEXT
    // CHECK_GLSL-DAG: hitObjectGetWorldToObjectEXT
    mat4x3 o2w = hitObjectGetObjectToWorldEXT(ho);
    mat4x3 w2o = hitObjectGetWorldToObjectEXT(ho);

    // CHECK_GLSL-DAG: hitObjectGetInstanceIdEXT
    // CHECK_GLSL-DAG: hitObjectGetInstanceCustomIndexEXT
    // CHECK_GLSL-DAG: hitObjectGetGeometryIndexEXT
    // CHECK_GLSL-DAG: hitObjectGetPrimitiveIndexEXT
    // CHECK_GLSL-DAG: hitObjectGetHitKindEXT
    int instIdx = hitObjectGetInstanceIdEXT(ho);
    int instCustomIdx = hitObjectGetInstanceCustomIndexEXT(ho);
    int geomIdx = hitObjectGetGeometryIndexEXT(ho);
    int primIdx = hitObjectGetPrimitiveIndexEXT(ho);
    uint hitKind = hitObjectGetHitKindEXT(ho);

    // CHECK_GLSL-DAG: hitObjectGetShaderBindingTableRecordIndexEXT
    // CHECK_GLSL-DAG: hitObjectSetShaderBindingTableRecordIndexEXT
    uint sbtIdx = hitObjectGetShaderBindingTableRecordIndexEXT(ho);
    hitObjectSetShaderBindingTableRecordIndexEXT(ho, 42u);

    // CHECK_GLSL: hitObjectGetShaderRecordBufferHandleEXT
    uvec2 sbtHandle = hitObjectGetShaderRecordBufferHandleEXT(ho);

    // CHECK_GLSL: hitObjectGetCurrentTimeEXT
    float currentTime = hitObjectGetCurrentTimeEXT(ho);

    // CHECK_GLSL: hitObjectGetIntersectionTriangleVertexPositionsEXT
    vec3 triVerts[3];
    hitObjectGetIntersectionTriangleVertexPositionsEXT(ho, triVerts);

    // CHECK_GLSL: reorderThreadWithHintEXT
    reorderThreadEXT(1u, 4u);

    // CHECK_GLSL: reorderThreadWithHitObjectEXT
    reorderThreadEXT(ho);

    // CHECK_GLSL: reorderThreadWithHitObjectAndHintEXT
    reorderThreadEXT(ho, 2u, 8u);

    // CHECK_GLSL: hitObjectReorderExecuteEXT
    hitObjectReorderExecuteEXT(ho, 0);

    // CHECK_GLSL: hitObjectReorderExecuteEXT
    hitObjectReorderExecuteEXT(ho, 1u, 4u, 0);

    // CHECK_GLSL: hitObjectTraceReorderExecuteEXT
    hitObjectTraceReorderExecuteEXT(ho, topLevel, 0u, 0xFFu, 0u, 0u, 0u,
        vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0, 0);

    // CHECK_GLSL: hitObjectTraceReorderExecuteEXT
    hitObjectTraceReorderExecuteEXT(ho, topLevel, 0u, 0xFFu, 0u, 0u, 0u,
        vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0, 1u, 4u, 0);

    // CHECK_GLSL: hitObjectTraceMotionReorderExecuteEXT
    hitObjectTraceMotionReorderExecuteEXT(ho, topLevel, 0u, 0xFFu, 0u, 0u, 0u,
        vec3(0.0), 0.001, vec3(0.0, 0.0, 1.0), 1000.0, 0.5, 1u, 4u, 0);
}
