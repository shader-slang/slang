//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute

// Two entry-point parameters that request the same (binding, set) must produce
// the explicit-binding overlap diagnostic, just like global parameters do. The
// entry-point binding path funnels through the same `addExplicitParameterBinding`
// used-range tracker, so this conflict is caught centrally.

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    [[vk::binding(0, 0)]]
    RWStructuredBuffer<float> a,
    [[vk::binding(0, 0)]]
    RWStructuredBuffer<float> b)
{
    a[0] = b[0];
}

// CHECK: warning[E39001]: explicit binding overlap
// CHECK: parameter 'b' overlaps with parameter 'a'
