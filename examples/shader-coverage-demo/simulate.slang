// simulate.slang — particle physics compute kernel entry point.
//
// The per-type physics helpers (`applyGravity`, `stepFluid`,
// `stepGas`, `stepSolid`) and the `Particle` struct live in
// `physics.slang` as a separate module. When `-trace-coverage` is
// on, counters are attributed to whichever file each statement
// came from, so the resulting LCOV report spans BOTH files — one
// `SF:` record per file, each with its own per-line hit counts.

import physics;

// Particle type discriminants. Matches the `PARTICLE_TYPE_*` enum
// on the host side.
static const uint PARTICLE_TYPE_FLUID = 0;
static const uint PARTICLE_TYPE_GAS   = 1;
static const uint PARTICLE_TYPE_SOLID = 2;

//TEST_INPUT: set particles = out ubuffer(data=[], stride=32)
RWStructuredBuffer<Particle> particles;

//TEST_INPUT: set params = ubuffer(data=[0], stride=4)
cbuffer Params
{
    uint  particleCount;
    float dt;
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
