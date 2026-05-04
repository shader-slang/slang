// physics.slang — physics helpers imported by app.slang. Imports
// math.slang in turn, so the demo exercises a three-module dependency
// chain: app → physics → math. The IR coverage pass dedupes the
// synthesized __slang_coverage buffer across all three so a single
// counter buffer covers every instrumented statement.

import math;

public float3 applyGravity(float3 velocity, float dt)
{
    float3 g = float3(0.0, -9.81, 0.0);
    return velocity + g * dt;
}

public float3 reflectOffGround(float3 velocity, float restitution)
{
    if (velocity.y >= 0.0)
    {
        // Unreachable on this demo's input pattern — applyGravity
        // always pulls velocity.y negative before reflectOffGround
        // is called. The 0-hit on this line shows the IR coverage
        // pass attributes counters even to never-executed branches.
        return velocity;
    }
    float3 reflected = float3(velocity.x, -velocity.y * restitution, velocity.z);
    return reflected;
}

public float3 applyDrag(float3 velocity, float drag)
{
    float factor = clamp01(1.0 - drag);
    return velocity * factor;
}
