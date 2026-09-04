//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Only an entry-point by-value uniform *aggregate* (struct / sized array) is address-forwarded. A
// scalar (`count`), a vector (`vec`) and a matrix (`mat`) - all passed by value into helpers
// alongside the aggregate - stay plain. The matrix matters most here: it is the composite type
// closest to a forwardable aggregate, so it is the shape a future edit to the type switch in
// `isEntryPointByValueUniformAggregateParam` is most likely to broaden into by accident.

struct Big
{
    float v[64];
}

// `count`/`vec`/`mat` are passed by value into helpers so the positive call-site checks below
// actually discriminate: a wrongly-broadened forward would turn them into `&count`/`&vec`/`&mat`.
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

[noinline]
float readmat(float4x4 m, uint i)
{
    return m[i][0];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform Big big,
    uniform uint count,
    uniform float4 vec,
    uniform float4x4 mat,
    uniform RWStructuredBuffer<float> output)
{
    if (tid.x < count)
        output[tid.x] =
            readsum(big, count, tid.x) + readvec(vec, tid.x % 4) + readmat(mat, tid.x % 4);
}

// Capture the aggregate, vector and matrix parameter names so the call-site checks below reference
// the exact same variables. Only the aggregate is address-forwarded (`&[[BIG_VAR]]`); the vector and
// matrix are passed by value (no `&`). These positive checks are the discriminating guard: had the
// forward wrongly broadened, `readvec_0`/`readmat_0` would emit `&[[VEC_VAR]]`/`&[[MAT_VAR]]` and
// fail to match.
// CHECK: __global__ void computeMain(Big_0 [[BIG_VAR:[A-Za-z0-9_]+]], uint {{[A-Za-z0-9_]+}}, float4 [[VEC_VAR:[A-Za-z0-9_]+]], {{.*}} [[MAT_VAR:mat_[0-9]+]]{{.*}}RWStructuredBuffer<float>{{.*}})
// CHECK: readsum_0(&[[BIG_VAR]], count_1
// CHECK: readvec_0([[VEC_VAR]],
// CHECK: readmat_0([[MAT_VAR]],
