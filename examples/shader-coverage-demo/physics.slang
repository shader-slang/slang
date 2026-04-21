// physics.slang — per-particle-type physics steps, split into its own
// module to demonstrate that `-trace-coverage` attributes counters
// correctly across multiple source files. The LCOV report produced
// by the demo will contain separate `SF:` records for this file and
// `simulate.slang`, each with its own per-line hit counts.

public struct Particle
{
    public float3 position;
    public float3 velocity;
    public uint   type;
    public uint   flags;
};

public float3 applyGravity(float3 velocity, float dt)
{
    return velocity + float3(0.0, -9.8, 0.0) * dt;
}

public void stepFluid(inout Particle p, float dt)
{
    p.velocity = applyGravity(p.velocity, dt);
    // Viscous drag proportional to speed.
    float speed = length(p.velocity);
    if (speed > 0.01)
        p.velocity -= normalize(p.velocity) * 0.5 * dt;
    p.position += p.velocity * dt;
}

public void stepGas(inout Particle p, float dt)
{
    // Gas rises buoyantly; no gravity pull, only ambient drag.
    p.velocity += float3(0.0, 2.5, 0.0) * dt;
    p.velocity *= 0.98;
    p.position += p.velocity * dt;
}

public void stepSolid(inout Particle p, float dt)
{
    // Solids fall under gravity; bounce on the floor.
    p.velocity = applyGravity(p.velocity, dt);
    p.position += p.velocity * dt;
    if (p.position.y < 0.0)
    {
        p.position.y = 0.0;
        p.velocity.y = -p.velocity.y * 0.6;  // inelastic bounce
    }
}
