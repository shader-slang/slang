// A module-scope `static const` vector or matrix whose initializer uses an operator, such as
// `int2(10, 7) / int2(4, 2)`, must compile for CUDA. No IR pass folds vector arithmetic, and the
// CUDA prelude implements the vector operators as non-`constexpr` functions, so a `__device__`
// variable that is initialized with one is a dynamic initializer that NVRTC rejects. Global
// value legalization therefore inlines such a value into the functions that use it.
// (shader-slang/slang#14330)

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type

static const int2 kDiv[2] = { int2(10, 7) / int2(4, 2), int2(1, 1) };
static const int2 kRem[2] = { int2(10, 7) % int2(4, 2), int2(1, 1) };
static const int2 kAdd[2] = { int2(10, 7) + int2(4, 2), int2(1, 1) };
static const int2 kMul[2] = { int2(10, 7) * int2(4, 2), int2(1, 1) };
static const float2 kFDiv[2] = { float2(10, 7) / float2(4, 2), float2(1, 1) };
static const int2 kSingle = int2(10, 7) / int2(4, 2);
static const int2 kScalar = int2(10, 7) / 2;
static const float2x2 kMatrix = float2x2(1, 2, 3, 4) * 2.0;

// No vector arithmetic is left in a `__device__` initializer.
// CUDA-NOT: __device__ static const
// CUDA: void computeMain(
// PTX: .entry computeMain

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // The index depends on the thread, so the array is not folded away.
    uint i = tid.x;

    // BUF: type: int32_t
    // BUF-NEXT: 2
    // BUF-NEXT: 3
    outputBuffer[0] = kDiv[i].x;
    outputBuffer[1] = kDiv[i].y;
    // BUF-NEXT: 2
    // BUF-NEXT: 1
    outputBuffer[2] = kRem[i].x;
    outputBuffer[3] = kRem[i].y;
    // BUF-NEXT: 14
    // BUF-NEXT: 9
    outputBuffer[4] = kAdd[i].x;
    outputBuffer[5] = kAdd[i].y;
    // BUF-NEXT: 40
    // BUF-NEXT: 14
    outputBuffer[6] = kMul[i].x;
    outputBuffer[7] = kMul[i].y;
    // BUF-NEXT: 2
    // BUF-NEXT: 3
    outputBuffer[8] = int(kFDiv[i].x * 1.0f);
    outputBuffer[9] = int(kFDiv[i].y * 1.0f);
    // BUF-NEXT: 2
    // BUF-NEXT: 3
    outputBuffer[10] = kSingle.x;
    outputBuffer[11] = kSingle.y;
    // BUF-NEXT: 5
    // BUF-NEXT: 3
    outputBuffer[12] = kScalar.x;
    outputBuffer[13] = kScalar.y;
    // BUF-NEXT: 2
    // BUF-NEXT: 4
    outputBuffer[14] = int(kMatrix[0][0]);
    outputBuffer[15] = int(kMatrix[0][1]);
    // BUF-NEXT: 6
    // BUF-NEXT: 8
    outputBuffer[16] = int(kMatrix[1][0]);
    outputBuffer[17] = int(kMatrix[1][1]);
}
