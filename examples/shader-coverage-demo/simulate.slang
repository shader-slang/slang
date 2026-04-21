// simulate.slang — particle physics step, with branching on particle
// type. The demo runs this kernel multiple times with different inputs
// so that coverage percentages differ between scenarios.

// Particle type discriminants. Matches the `PARTICLE_TYPE_*` enum on
// the host side.
static const uint PARTICLE_TYPE_FLUID = 0;
static const uint PARTICLE_TYPE_GAS   = 1;
static const uint PARTICLE_TYPE_SOLID = 2;

struct Particle
{
    float3 position;
    float3 velocity;
    uint   type;
    uint   flags;
};

//TEST_INPUT: set particles = out ubuffer(data=[], stride=32)
RWStructuredBuffer<Particle> particles;

//TEST_INPUT: set params = ubuffer(data=[0], stride=4)
cbuffer Params
{
    uint  particleCount;
    float dt;
}

float3 applyGravity(float3 velocity, float dt)
{
    return velocity + float3(0.0, -9.8, 0.0) * dt;
}

void stepFluid(inout Particle p, float dt)
{
    p.velocity = applyGravity(p.velocity, dt);
    // Viscous drag proportional to speed.
    float speed = length(p.velocity);
    if (speed > 0.01)
        p.velocity -= normalize(p.velocity) * 0.5 * dt;
    p.position += p.velocity * dt;
}

void stepGas(inout Particle p, float dt)
{
    // Gas rises buoyantly; no gravity pull, only ambient drag.
    p.velocity += float3(0.0, 2.5, 0.0) * dt;
    p.velocity *= 0.98;
    p.position += p.velocity * dt;
}

void stepSolid(inout Particle p, float dt)
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

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    if (i >= particleCount)
        return;

    Particle p = particles[i];

    if (p.type == PARTICLE_TYPE_FLUID)
    {
        stepFluid(p, dt);
    }
    else if (p.type == PARTICLE_TYPE_GAS)
    {
        stepGas(p, dt);
    }
    else if (p.type == PARTICLE_TYPE_SOLID)
    {
        stepSolid(p, dt);
    }
    else
    {
        // Unknown type — flag for host-side triage. This branch is
        // intentionally unreachable in the "normal" test scenarios;
        // the coverage report will show it uncovered, demonstrating
        // that the instrumentation spots dead code.
        p.flags |= 0x1u;
    }

    particles[i] = p;
}
