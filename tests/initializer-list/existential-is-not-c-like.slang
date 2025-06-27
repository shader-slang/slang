// Test that in Slang 2026, it is no longer valid to default initialize an existential value.
#lang 2026

//TEST:SIMPLE(filecheck=CHECK): -target spirv -std 2026
interface IBSDF
{
    float3 eval(float3 wi, float3 wo);
}

struct ShaderGraph
{
    // CHECK: ([[# @LINE+1]]): error
    IBSDF bsdf_stack[8]; // Intentionally uninitialized.
    int next_bsdf = 0; // must be zero.
}

[numthreads(1,1,1)]
void main()
{
    ShaderGraph sg = {};
}