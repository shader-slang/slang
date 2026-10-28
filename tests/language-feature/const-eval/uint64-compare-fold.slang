// Comparisons on integer constants must fold by the values the operands have at their types
// (#13978).
//
// Integer constants are held in a signed 64-bit integer, so a `uint64_t` above INT64_MAX such as
// 0xFFFFFFFFFFFFFFFF is stored as -1. Each folder below must still order it above 5: the IR SCCP
// pass, the front-end folds that size arrays, and the folds that run once a generic value
// parameter is specialized. Front-end folding does not narrow results, so wrapped 32-bit results
// pin the conversion to the operand's width; ordinary signed and 32-bit comparisons are controls.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const uint64_t kMax = 0xFFFFFFFFFFFFFFFFull;
static const uint64_t kTop = 0x8000000000000000ull;

int arraySizeInGeneric<let N : uint64_t>()
{
    int a[(N > 5ull) ? 1 : 2];
    return a.getCount();
}

// Packs all four orderings against 5 into one value: 3 means N > 5, 12 means N < 5.
int compareInGeneric<let N : uint64_t>()
{
    return ((N > 5ull) ? 1 : 0) + ((N >= 5ull) ? 2 : 0) + ((N < 5ull) ? 4 : 0) +
           ((N <= 5ull) ? 8 : 0);
}

// The array size is folded when the generic type is specialized.
struct SizedByComparison<let N : uint64_t>
{
    int a[(N > 5ull) ? 1 : 2];
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // IR SCCP on local values.
    uint64_t x = 0xFFFFFFFFFFFFFFFFull;
    outputBuffer[0] = (x > 5ull) ? 1 : 0;
    outputBuffer[1] = (x >= 5ull) ? 1 : 0;
    outputBuffer[2] = (x < 5ull) ? 1 : 0;
    outputBuffer[3] = (x <= 5ull) ? 1 : 0;
    outputBuffer[4] = (5ull < x) ? 1 : 0;
    outputBuffer[5] = (0x8000000000000000ull > 1ull) ? 1 : 0;

    // Front-end folding of array sizes through the builtin-operator expression.
    int a0[(kMax > 5ull) ? 1 : 2];
    int a1[(kMax >= 5ull) ? 1 : 2];
    int a2[(kMax < 5ull) ? 1 : 2];
    int a3[(kTop <= 1ull) ? 1 : 2];
    outputBuffer[6] = a0.getCount();
    outputBuffer[7] = a1.getCount();
    outputBuffer[8] = a2.getCount();
    outputBuffer[9] = a3.getCount();

    // Front-end folding of an explicit operator call, which is not rewritten to a
    // builtin-operator expression.
    int a4[operator>(kMax, 5ull) ? 1 : 2];
    outputBuffer[10] = a4.getCount();

    // Generic value parameter, folded after specialization.
    outputBuffer[11] = arraySizeInGeneric<0xFFFFFFFFFFFFFFFFull>();
    outputBuffer[12] = compareInGeneric<0xFFFFFFFFFFFFFFFFull>();
    outputBuffer[13] = compareInGeneric<0x8000000000000000ull>();
    outputBuffer[14] = compareInGeneric<3ull>();
    SizedByComparison<0xFFFFFFFFFFFFFFFFull> sized;
    outputBuffer[15] = sized.a.getCount();

    // Front-end folding does not narrow a wrapped 32-bit result, so `0u - 1u` is held as -1 and
    // `1u - 0xFFFFFFFFu` as -4294967294. Slot 16 needs an unsigned compare; slot 17 also needs the
    // operand converted to `uint` width, which gives 2.
    int w0[((0u - 1u) > 5u) ? 1 : 2];
    int w1[((1u - 0xFFFFFFFFu) < 10u) ? 1 : 2];
    outputBuffer[16] = w0.getCount();
    outputBuffer[17] = w1.getCount();

    // Signed int64 controls: negative values still order below zero.
    int64_t m = -1;
    outputBuffer[18] = (m < 0LL) ? 1 : 0;
    outputBuffer[19] = (int64_t(-0x8000000000000000LL) < 0LL) ? 1 : 0;
    int s0[(-1 < 0) ? 1 : 2];
    outputBuffer[20] = s0.getCount();
    int s1[operator<(-1, 0) ? 1 : 2];
    outputBuffer[21] = s1.getCount();
    // `uint` converts to the wider `int64_t`, so this comparison is signed.
    int s2[(int64_t(-1) < 5u) ? 1 : 2];
    outputBuffer[22] = s2.getCount();

    // uint32 controls.
    uint u = 0xFFFFFFFFu;
    outputBuffer[23] = (u > 5u) ? 1 : 0;
    int s3[(0xFFFFFFFFu > 5u) ? 1 : 2];
    outputBuffer[24] = s3.getCount();

    // Equality reads the same wrapped value as ordering does, on both front-end entry points.
    int e0[((0u - 1u) == 0xFFFFFFFFu) ? 1 : 2];
    int e1[operator!=(0u - 1u, 0xFFFFFFFFu) ? 1 : 2];
    outputBuffer[25] = e0.getCount();
    outputBuffer[26] = e1.getCount();

    // A wrapped `int` result orders by its `int` value, as it does at run time.
    int s4[((2147483647 + 1) < 0) ? 1 : 2];
    outputBuffer[27] = s4.getCount();
}

// CHECK: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 2
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 3
// CHECK-NEXT: 3
// CHECK-NEXT: 12
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 1
