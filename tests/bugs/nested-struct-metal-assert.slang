// Test that nested differentiable struct instances with mixed resource/value
// fields compile correctly for Metal target.
// Regression test for assertion failure in type legalization (PR #9808).

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry main

struct Material : IDifferentiable {
    float3 color;
    float3 emission;

    [Differentiable]
    __init() {
        color = float3(1, 1, 1);
        emission = float3(0, 0, 0);
    }
};

struct Particle : IDifferentiable {
    float2 position;
    float2 velocity;
    float size;
    Material material;

    [Differentiable]
    __init(float2 pos, float2 vel) {
        this.position = pos;
        this.velocity = vel;
        this.material = Material();
        this.size = 0.5;
    }
};

void storeParticle(
    RWStructuredBuffer<float2> positions,
    RWStructuredBuffer<float2> velocities,
    RWStructuredBuffer<float> sizes,
    RWStructuredBuffer<Material> materials,
    uint idx,
    Particle p)
{
    positions[idx] = p.position;
    velocities[idx] = p.velocity;
    sizes[idx] = p.size;
    materials[idx] = p.material;
}

// CHECK: main
[numthreads(1, 1, 1)]
void main(
    uniform RWStructuredBuffer<float2> positions,
    uniform RWStructuredBuffer<float2> velocities,
    uniform RWStructuredBuffer<float> sizes,
    uniform RWStructuredBuffer<Material> materials,
    uint3 tid : SV_DispatchThreadID)
{
    Particle p = Particle(float2(1, 2), float2(3, 4));
    storeParticle(positions, velocities, sizes, materials, tid.x, p);
}
