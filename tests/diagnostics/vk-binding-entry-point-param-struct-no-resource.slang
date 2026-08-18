// Negative counterpart to the #11861 fix: a `[[vk::binding(...)]]` on an entry-point
// parameter whose struct type consumes NO descriptor-shaped resource is genuinely
// ignored by the binder, so the E38010 "unsupported ... will be ignored" warning must
// still fire. This guards against over-broadening the struct recursion in
// `isVkBindingCompatibleEntryPointParameterType`.
//
// Two shapes are covered:
//   - `PlainData`: only plain-data fields, no resource of any kind.
//   - `PtrData`: only a raw pointer field. A raw pointer is a buffer-device-address
//     value with no descriptor slot to position, so the binder never honors a binding
//     on it (this is the #11857 leaf case). The struct recursion must NOT suppress the
//     warning just because a field exists; it must find a genuine descriptor-consuming
//     leaf, and a pointer is not one.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

RWStructuredBuffer<uint> output;

struct PlainData
{
    float4 a;
    float4 b;
};

struct PtrData
{
    uint* p;
};

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(1, 0)]] uniform PlainData data,
    [[vk::binding(2, 0)]] uniform PtrData ptrs)
{
    output[0] = uint(data.a.x + data.b.y) + ptrs.p[0];
}
//CHECK: E38010
//CHECK: on parameter 'data' is unsupported on entry point parameters and will be ignored
//CHECK: E38010
//CHECK: on parameter 'ptrs' is unsupported on entry point parameters and will be ignored
