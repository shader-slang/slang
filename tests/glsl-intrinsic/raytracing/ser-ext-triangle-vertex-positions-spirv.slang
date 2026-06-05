// Regression test: SPIR-V emission of the GLSL-style
// hitObjectGetIntersectionTriangleVertexPositionsEXT free function.
//
// Opcode 5307 (OpHitObjectGetIntersectionTriangleVertexPositionsEXT) is a
// result-producing instruction: IdResultType + IdResult + HitObject, returning
// an array of 3 vec3 (matching OpRayQueryGetIntersectionTriangleVertexPositionsKHR).
// It was previously emitted incorrectly as a void instruction writing through a
// pointer operand, which produces invalid SPIR-V. This locks in the correct form.

//TEST:SIMPLE(filecheck=CHECK): -allow-glsl -stage raygeneration -entry main -target spirv-asm -capability spvShaderInvocationReorderEXT

import glsl;

layout(binding = 0) uniform accelerationStructureEXT topLevel;
layout(binding = 1) buffer Output { float result; } outBuf;

// The instruction must produce a result id ("%n =") of array-of-vec3 type, not a
// bare void instruction. The "%n =" prefix is what distinguishes the fixed form.
// CHECK-DAG: OpExtension "SPV_KHR_ray_tracing_position_fetch"
// CHECK-DAG: OpCapability RayTracingPositionFetchKHR
// CHECK: {{%[A-Za-z0-9_]+}} = OpHitObjectGetIntersectionTriangleVertexPositionsEXT {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}}

void main()
{
    hitObjectEXT ho;
    vec3 triVerts[3];
    hitObjectGetIntersectionTriangleVertexPositionsEXT(ho, triVerts);
    outBuf.result = triVerts[0].x + triVerts[1].y + triVerts[2].z;
}
