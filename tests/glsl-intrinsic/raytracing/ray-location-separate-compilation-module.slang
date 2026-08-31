layout(binding = 0) uniform accelerationStructureEXT scene;
layout(location = 2) rayPayloadEXT vec4 payload;

// Keep a payload-by-location operand live in the serialized function body for the final link.
void traceWithSeparatelyCompiledPayload()
{
    traceRayEXT(
        scene,
        gl_RayFlagsNoneEXT,
        0xff,
        0,
        0,
        0,
        vec3(0.0),
        0.001,
        vec3(0.0, 0.0, 1.0),
        1000.0,
        2);
}
