//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute

// An entry-point parameter binding that collides with a global parameter binding
// at the same (set, binding) must be diagnosed. Explicit entry-point bindings are
// reserved after the global parameters in the same used-range tracker, so the
// overlap is detected centrally rather than silently stomping the descriptor slot.

[[vk::binding(0, 1)]]
Texture2D g;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    [[vk::binding(0, 1)]]
    RWStructuredBuffer<float> b)
{
    b[0] = g.Load(int3(0, 0, 0)).x;
}

// CHECK: warning[E39001]: explicit binding overlap
// CHECK: parameter 'b' overlaps with parameter 'g'
