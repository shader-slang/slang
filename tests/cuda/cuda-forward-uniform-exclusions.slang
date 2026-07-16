//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Only an entry-point by-value uniform *aggregate* is address-forwarded: a scalar (`count`) and a
// vector (`vec`), passed by value into helpers alongside the aggregate, stay plain.

struct Big
{
    float v[64];
}

// `count`/`vec` are helper arguments so the CHECK-NOTs below are non-vacuous.
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
// CHECK: readsum_0((&(big_0)), count_1
// CHECK: readvec_0(vec_0
// CHECK-NOT: (&(count_0))
// CHECK-NOT: (&(count_1))
// CHECK-NOT: (&(vec_0))
