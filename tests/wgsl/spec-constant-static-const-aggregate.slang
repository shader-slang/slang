//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
// The NOCONST run has no positive check, so its `NOCONST-NOT` scans the whole output.
//TEST:SIMPLE(filecheck=NOCONST): -target wgsl -entry computeMain -stage compute
// No wgsl-spirv-asm run: the bundled slang-tint cannot compile override-expressions such as a
// `select` condition over an `override`, so the WebGPU run validates the emitted WGSL instead.
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// Aggregates with an element that depends on a specialization constant cannot be a WGSL
// `const`, so they are built in the function that uses them, and a function-local value
// over an `override` is declared as a `var`.

[vk::constant_id(0)]
const int N = 4;

static const int k = 64 + N;

static const int arr[2] = { k, 3 };

// Member-wise struct initializers lower to module-scope `MakeStruct`. The fields of `s` fold
// at their uses; `ss` is indexed at run time, so its structs must be built in the function.
struct S
{
    int a;
    int b;
}
static const S s = { 1, k };
static const S ss[2] = { { 1, k }, { 2, 3 } };

// `v` and `m` are read through swizzled stores, and WGSL keeps the right-hand side of a
// swizzled store in a temporary, which must not be a `const`.
static const int2 v = int2(k, 1);
static const int2x2 m = int2x2(k, 1, 2, 3);
// `ms` reaches MakeMatrixFromScalar directly, and `vb` below reaches MakeVectorFromScalar
// inside an array that must be inlined with it.
static const int2x2 ms = int2x2(k);

// An inner vector or array that is inlined must make the array that holds it inline too.
static const int2 vecs[2] = { int2(k, 1), int2(2, 3) };
static const int grid[2][2] = { { k, 1 }, { 2, 3 } };
static const int2 vb[2] = { int2(k), int2(2, 3) };

// WGSL rejects a `const` whose value depends on an `override`.
// NOCONST-NOT: const {{.*}}N_0

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=16):out,name=outputBuffer
RWStructuredBuffer<int4> outputBuffer;

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // CHECK: fn computeMain
    // CHECK-DAG: array<i32, i32(2)>( i32(64) + N_0, i32(3) )
    // CHECK-DAG: S_0( i32(1), i32(64) + N_0 )
    // CHECK-DAG: var ss_0 : array<S_0, i32(2)> = array<S_0, i32(2)>(
    // CHECK-DAG: var {{.*}} : vec2<i32> = vec2<i32>(i32(64) + N_0, i32(1));
    // CHECK-DAG: var vecs_0 : array<vec2<i32>, i32(2)> = array<vec2<i32>, i32(2)>(
    // CHECK-DAG: var grid_0 : array<array<i32, i32(2)>, i32(2)> = array<array<i32, i32(2)>, i32(2)>(
    uint t = tid.x;
    outputBuffer[t].x = arr[t];
    outputBuffer[t].y = t == 0 ? s.a : s.b;
    outputBuffer[t].zw = vecs[t];
    outputBuffer[2 + t].xy = int2(grid[t][0], grid[t][1]);
    outputBuffer[6 + t].xy = vb[t];
    outputBuffer[8 + t].xy = int2(ss[t].a, ss[t].b);
    if (t == 0)
    {
        outputBuffer[4].xy = v;
        outputBuffer[4].zw = m[1];
        outputBuffer[5].xy = m[0];
        // A function-local vector over the `override` itself must not be declared `const`.
        outputBuffer[5].zw = int2(N, 1);
        outputBuffer[6].zw = ms[1];
    }
}

// One line per component of each int4 element: [0] and [1] hold arr, s and vecs for each
// thread; [2] and [3] hold the grid rows; [4] holds v and m[1]; [5] holds m[0] and
// int2(N, 1); [6] and [7] hold vb for each thread, with ms[1] in [6].zw; [8] and [9] hold
// ss for each thread.
// BUF: 68
// BUF-NEXT: 1
// BUF-NEXT: 68
// BUF-NEXT: 1
// BUF-NEXT: 3
// BUF-NEXT: 68
// BUF-NEXT: 2
// BUF-NEXT: 3
// BUF-NEXT: 68
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 2
// BUF-NEXT: 3
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 68
// BUF-NEXT: 1
// BUF-NEXT: 2
// BUF-NEXT: 3
// BUF-NEXT: 68
// BUF-NEXT: 1
// BUF-NEXT: 4
// BUF-NEXT: 1
// BUF-NEXT: 68
// BUF-NEXT: 68
// BUF-NEXT: 68
// BUF-NEXT: 68
// BUF-NEXT: 2
// BUF-NEXT: 3
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 68
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 2
// BUF-NEXT: 3
// BUF-NEXT: 0
// BUF-NEXT: 0
