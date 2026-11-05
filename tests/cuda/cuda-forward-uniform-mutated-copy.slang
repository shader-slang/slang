//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RT):-cuda -compute -output-using-type -shaderobj

// Mutation-path: a helper that "mutates" its by-value uniform aggregate. The callee copies the
// pointee (`*b_0`) into a local before writing, so the forwarded address is only read - which is why
// the forward is sound with no qualifier or const_cast. The whole PR's correctness rests on "no write
// reaches the forwarded parameter storage", so the runtime half proves it directly: the kernel calls
// the mutating helper and then re-reads the *original* uniform (`data.v[0]`); if a write had leaked to
// the forwarded storage, that re-read would be clobbered. (COMPARE_COMPUTE is GPU-gated - `ignored`
// on a GPU-less runner, executed on the GPU lane.)

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set data.v = {10.0, 20.0, 30.0, 40.0}

struct Big
{
    float v[4];
}

[noinline]
float mutate_and_read(Big b, uint i)
{
    b.v[0] = 999.0f;
    return b.v[i];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform Big data,
    uniform RWStructuredBuffer<float> output)
{
    // The helper clobbers v[0]=999 in its OWN copy, returns copy.v[i]; the kernel then reads the
    // original `data.v[0]`, which must still be 10 in every lane - proving the write never reached
    // the forwarded storage. out[i] = mutate_and_read(data, i) + data.v[0]:
    //   i0: returns clobbered copy.v[0]=999, + orig 10 => 1009 (the mutation did happen, on the copy);
    //   i1..i3: returns unclobbered copy.v[i], + orig 10 => 30, 40, 50.
    // CHECK_RT: 1009.0
    // CHECK_RT: 30.0
    // CHECK_RT: 40.0
    // CHECK_RT: 50.0
    output[tid] = mutate_and_read(data, tid) + data.v[0];
}

// Helper copies the pointee into a local before the write, so the write never reaches the pointer.
// CHECK: mutate_and_read_0(Big_0 * b_0
// CHECK: Big_0 {{.*}} = *b_0;
// CHECK: __global__ void computeMain(Big_0 [[BIG:data_0]], RWStructuredBuffer<float>
// CHECK: mutate_and_read_0(&[[BIG]]
