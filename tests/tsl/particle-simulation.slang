//TEST:SIMPLE(filecheck=TSL): -target tsl -stage compute -entry updateParticles

// Test TSL output for particle simulation patterns
// This test verifies TSL support for:
// - Storage buffers with structured data
// - Basic arithmetic operations

// Check TSL imports
// TSL: import {
// TSL: } from 'three/tsl';

// Check entry point is exported
// TSL: export const updateParticles = /*@__PURE__*/ Fn

// Check arithmetic operations use TSL function calls
// TSL: add(
// TSL: mul(

// Particle data structure
struct Particle
{
    float3 position;
    float3 velocity;
    float mass;
};

// Storage buffers for particle data
RWStructuredBuffer<Particle> particles;
RWStructuredBuffer<float> outputData;

[numthreads(64, 1, 1)]
void updateParticles(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;

    // Read particle data
    Particle p = particles[idx];

    // Simple physics update
    float dt = 0.016f; // ~60fps
    float3 gravity = float3(0.0f, -9.81f, 0.0f);

    // Update velocity: v = v + a * dt
    p.velocity = p.velocity + gravity * dt;

    // Update position: p = p + v * dt
    p.position = p.position + p.velocity * dt;

    // Write back
    particles[idx] = p;

    // Output speed squared (avoid length() for now)
    float speedSq = p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y + p.velocity.z * p.velocity.z;
    outputData[idx] = speedSq;
}
