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

// CHECK: __global__ void computeMain(Big_0 {{[A-Za-z0-9_]+}}, uint {{[A-Za-z0-9_]+}}, float4 {{.*}}RWStructuredBuffer<float>{{.*}})
// Only the aggregate is address-forwarded; the scalar and vector are passed by value. These
// positive call-site checks are the discriminating guard: had the forward wrongly broadened to
// scalars/vectors, the emitted argument would be `&count_1` / `&vec_0` and these matches would fail.
// CHECK: readsum_0(&big_0, count_1
// CHECK: readvec_0(vec_0
