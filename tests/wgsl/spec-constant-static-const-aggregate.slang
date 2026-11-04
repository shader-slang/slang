//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOCONST): -target wgsl -entry computeMain -stage compute
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// A module-scope aggregate with an element that depends on a specialization
// constant cannot be a WGSL `const`, so it is built in the function that uses it.

[vk::constant_id(0)]
const int N = 4;

static const int k = 64 + N;
static const int arr[2] = { k, 3 };

struct S
{
    int a;
    int b;
}
static const S s = { 1, k };

// WGSL rejects a `const` whose value depends on an `override`.
// NOCONST-NOT: const {{.*}}N_0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // CHECK: fn computeMain
    // CHECK-DAG: array<i32, i32(2)>( i32(64) + N_0, i32(3) )
    // CHECK-DAG: S_x24init{{.*}}(i32(1), i32(64) + N_0)
    outputBuffer[tid.x] = arr[tid.x];
    outputBuffer[tid.x + 2] = tid.x == 0 ? s.a : s.b;
}

// BUF: 68
// BUF-NEXT: 3
// BUF-NEXT: 1
// BUF-NEXT: 68
