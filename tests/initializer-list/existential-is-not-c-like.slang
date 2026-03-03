// Test that in Slang 2026, it is no longer valid to default initialize an existential value.
#lang 2026

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv
interface IBSDF
{
    float3 eval(float3 wi, float3 wo);
}

struct ShaderGraph
{
    IBSDF bsdf_stack[8]; // Intentionally uninitialized.
    int next_bsdf = 0; // must be zero.
}

[numthreads(1,1,1)]
void main()
{
    ShaderGraph sg = {};
//CHECK:             ^ not enough arguments to call
//CHECK:             ^ not enough arguments to call (got 0, expected 1)
}