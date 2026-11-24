// Companion to gh-14345-groupshared-load-before-barrier.slang. A call with side effects may
// change memory reached through a pointer, but not memory passed to the function as `out`,
// `inout` or `__borrow`: the caller gives the function exclusive access to that memory for
// the whole call. So a store through an `inout` parameter is still forwarded to a later load
// across such a call, and the value stored is returned directly.

//TEST:SIMPLE(filecheck=METAL): -target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry main -stage compute

RWStructuredBuffer<float> io;

[noinline]
void bump(uint i)
{
    io[i] = io[i] + 1.0f;
}

// METAL-LABEL: float storeThenCall_0(
// METAL: *x_0 = v_0;
// METAL: bump_0(
// METAL-NOT: *x_0
// METAL: return v_0;
// CUDA-LABEL: float storeThenCall_0(
// CUDA: *x_0 = v_0;
// CUDA-NEXT: bump_0(
// CUDA-NEXT: return v_0;
float storeThenCall(inout float x, float v, uint i)
{
    x = v;
    bump(i);
    return x;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    float y = io[tid.x];
    float r = storeThenCall(y, io[tid.x + 64], tid.x);
    io[tid.x] = r + y;
}
