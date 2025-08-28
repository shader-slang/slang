//TEST:COMPILE:-target wgsl -entry computeMain

// Regression test for issue #8184
// Test that accessing static const interface fields through generic type parameters
// does not crash the compiler.

interface Transformation
{
    float3 apply(float3 coord);
    static const uint32_t kParamCount;
}

struct T1 : Transformation {
    float3 apply(float3 coord) { return float3(0, 0, 0); }
    static const uint32_t kParamCount = 2;
};

// Test the case that should work
struct UseConstInStruct<T : Transformation>
{
    static const uint32_t kParamCount = T.kParamCount;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // This should work without crashing
    static const uint32_t count = UseConstInStruct<T1>.kParamCount;
}