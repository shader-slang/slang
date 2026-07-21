//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// The same by-value uniform aggregate is forwarded by address into two different borrow-in helpers
// and also read inline in the kernel. This exercises the two subtlest branches of the forward:
//   1. idempotency - `big` is retyped to `PhysicalParamStorage<T>` on the first forward; the second
//      forward must reuse the pointer (not re-retype or spill a copy), so both call sites emit `&big`.
//   2. the inline-read redirect - `big.v[0]` is a non-call value use, so the forward inserts a single
//      load of the parameter and reads the field off that (a missed load would read a pointer as a value).

struct Big
{
    float v[128];
}

// [noinline] keeps each kernel->helper boundary so the aggregate is forwarded, not inlined.
[noinline]
float readsum(Big b, uint i)
{
    return b.v[i] + b.v[i + 1];
}

[noinline]
float readmax(Big b, uint i)
{
    return b.v[i] > b.v[i + 1] ? b.v[i] : b.v[i + 1];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform Big big,
    uniform StructuredBuffer<uint> indices,
    uniform RWStructuredBuffer<float> output)
{
    uint i = indices[tid.x];
    output[tid.x] = readsum(big, i) + readmax(big, i) + big.v[0];
}

// Kernel ABI unchanged: the aggregate is still a plain by-value kernel argument.
// CHECK: __global__ void computeMain(Big_0 [[BIG:big_0]], StructuredBuffer<uint>

// The inline read goes through one load of the parameter, not the pointer.
// CHECK: Big_0 {{[A-Za-z0-9_]+}} = [[BIG]];

// Both helpers receive the parameter's address; no per-thread whole-aggregate copy is spilled.
// CHECK-DAG: readsum_0(&[[BIG]]
// CHECK-DAG: readmax_0(&[[BIG]]
