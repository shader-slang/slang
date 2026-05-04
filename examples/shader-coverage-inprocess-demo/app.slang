// app.slang — entry-point module. Imports physics.slang and math.slang
// (and physics.slang in turn imports math.slang) to demonstrate that
// Slang's shader coverage instrumentation handles multi-module compiles
// end-to-end: counters are attributed to the right file:line regardless
// of which imported module the statement lives in.

import math;
import physics;

RWStructuredBuffer<float3> positions;
RWStructuredBuffer<float3> velocities;

[shader("compute")]
[numthreads(1, 1, 1)]
void simulate(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    float3 pos = positions[i];
    float3 vel = velocities[i];

    vel = applyGravity(vel, 0.016);

    if (pos.y < 0.0)
    {
        vel = reflectOffGround(vel, 0.8);
        pos.y = 0.0;
    }

    vel = applyDrag(vel, 0.02);
    pos = pos + vel * 0.016;

    float3 dir = safeNormalize(vel);

    positions[i] = pos;
    velocities[i] = vel;
}
