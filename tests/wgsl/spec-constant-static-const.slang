//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOCONST): -target wgsl -entry computeMain -stage compute
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type

// A module-scope `static const` whose initializer uses a specialization constant
// is itself a specialization-constant value. On WGSL it stays at module scope and
// is folded into each use as an expression over the `override` declaration.

[vk::constant_id(0)]
const int N = 4;

static const int k = 64 + N;
static const int k2 = 64 + N;
static const int s = N > 0 ? 10 : 20;
static const uint u = N;
static const bool b = N > 0;
static const int sh = (N << 1) + 3;

// WGSL rejects a `const` whose value depends on an `override`.
// NOCONST-NOT: const {{.*}}N_0

// CHECK: @id(0) override N{{.*}} : i32 = i32(4);
// CHECK: fn f{{.*}}
// CHECK: return i32(64) + N_0;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int f()
{
    return k;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: computeMain
    // CHECK: = i32(64) + N_0;
    outputBuffer[0] = k;
    outputBuffer[1] = f();
    outputBuffer[2] = k2;
    outputBuffer[3] = k * 2;
    outputBuffer[4] = k + k2;
    outputBuffer[5] = s;
    outputBuffer[6] = int(u);
    outputBuffer[7] = b ? 1 : 0;
    // CHECK: [i32(8)] = {{.*}}N_0 << (u32(1)){{.*}} + i32(3);
    outputBuffer[8] = sh;
}

// BUF: 68
// BUF-NEXT: 68
// BUF-NEXT: 68
// BUF-NEXT: 136
// BUF-NEXT: 136
// BUF-NEXT: 10
// BUF-NEXT: 4
// BUF-NEXT: 1
// BUF-NEXT: 11
