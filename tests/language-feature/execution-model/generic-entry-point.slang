// Test that a non-generic entry point calling a generic helper
// function lowers correctly. Slang's frontend implicitly
// specializes the helper against the inferred type. This exercises
// the specialization-argument expansion machinery in
// `slang-check-shader.cpp` and the surrounding generic-resolution
// paths that the PR's other entry-point tests don't reach.
//
// The PR's other tests use only concrete-typed call chains; this
// test pins the implicit-specialization path through compute.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

interface IAddable
{
    int add(int x);
}

struct Plus2 : IAddable
{
    int add(int x) { return x + 2; }
};

// Generic helper. The entry point implicitly specializes this
// against `Plus2` (the only IAddable in scope).
int callAdder<T : IAddable>(T t, int x)
{
    return t.add(x);
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Plus2 t;
    outputBuffer[tid.x] = callAdder(t, int(tid.x));
}

// Plus2.add(x) returns x + 2, so the buffer becomes [2, 3, 4, 5].
// Use {{^N$}} anchors to disambiguate from "int32_t" (which
// contains a "2") in the type-prefix line.
// CHECK: {{^2$}}
// CHECK: {{^3$}}
// CHECK: {{^4$}}
// CHECK: {{^5$}}
