//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Only an entry-point by-value uniform *aggregate* is address-forwarded: a scalar (`count`) and a
// vector (`vec`), passed by value into helpers alongside the aggregate, stay plain.

struct Big
{
    float v[64];
}

// `count`/`vec` are passed by value into helpers so the positive call-site checks below actually
// discriminate: a wrongly-broadened forward would turn them into `&count`/`&vec` there.
[noinline]
float readsum(Big b, uint count, uint i)
{
    return b.v[i] + (float)count;
}

[noinline]
float readvec(float4 v, uint i)
{
    return v[i];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform Big big,
    uniform uint count,
    uniform float4 vec,
    uniform RWStructuredBuffer<float> output)
{
    if (tid.x < count)
        output[tid.x] = readsum(big, count, tid.x) + readvec(vec, tid.x % 4);
}

// Capture the aggregate and vector parameter names so the call-site checks below reference the
// exact same variables. Only the aggregate is address-forwarded (`&[[BIG_VAR]]`); the vector is
// passed by value (`[[VEC_VAR]]`, no `&`). These positive checks are the discriminating guard: had
// the forward wrongly broadened to vectors, `readvec_0` would emit `&[[VEC_VAR]]` and fail to match.
// CHECK: __global__ void computeMain(Big_0 [[BIG_VAR:[A-Za-z0-9_]+]], uint {{[A-Za-z0-9_]+}}, float4 [[VEC_VAR:[A-Za-z0-9_]+]]{{.*}}RWStructuredBuffer<float>{{.*}})
// CHECK: readsum_0(&[[BIG_VAR]], count_1
// CHECK: readvec_0([[VEC_VAR]],
