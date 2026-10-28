// Folding a 64-bit `/` or `%` whose divisor is -1 must not divide on the host (#13986).
//
// `INT64_MIN / -1` overflows, and the host's signed division traps on it (SIGFPE on x86-64).
// The IR SCCP pass and the front-end folds must fold the wrapped quotient INT64_MIN and the
// remainder 0 instead. Integer constants are held in a signed 64-bit integer, so the `uint64_t`
// divisor 0xFFFFFFFFFFFFFFFF is held as -1 as well; it must divide as unsigned, which gives
// 0x8000000000000000 / 0xFFFFFFFFFFFFFFFF == 0 and a remainder of 0x8000000000000000. Signed
// `-7 / 2` and `-7 % 2`, 32-bit `INT_MIN / -1` and `% -1`, and 32-bit unsigned `~0u / 1u` and
// `~0x7FFFFFFFu % 0xFFFFFFFFu` are controls whose results must not change.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=8, count=25):out,name=outputBuffer
RWStructuredBuffer<int64_t> outputBuffer;

static const int64_t kMin = -9223372036854775807ll - 1ll;
static const uint64_t kTop = 0x8000000000000000ull;
static const uint64_t kMax = 0xFFFFFFFFFFFFFFFFull;

// The array sizes below lower to `constexprDiv` and `constexprIRem` IR.
int sizeFromSignedGeneric<let N : int64_t>()
{
    int a[(N / -1ll == N && N % -1ll == 0ll) ? 1 : 2];
    return a.getCount();
}

int sizeFromUnsignedGeneric<let N : uint64_t>()
{
    int a[(N / kMax == 0ull && N % kMax == N) ? 1 : 2];
    return a.getCount();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: type: int64_t

    // IR SCCP on local values.
    int64_t a = -9223372036854775807ll - 1ll;
    int64_t b = -1ll;
    outputBuffer[0] = a / b; // CHECK-NEXT: -9223372036854775808
    outputBuffer[1] = a % b; // CHECK-NEXT: 0
    uint64_t ua = 0x8000000000000000ull;
    uint64_t ub = 0xFFFFFFFFFFFFFFFFull;
    outputBuffer[2] = int64_t(ua / ub);   // CHECK-NEXT: 0
    outputBuffer[3] = int64_t(ua % ub);   // CHECK-NEXT: -9223372036854775808
    outputBuffer[4] = int64_t(ub / 2ull); // CHECK-NEXT: 9223372036854775807
    int64_t c = -7ll;
    int64_t d = 2ll;
    outputBuffer[5] = c / d; // CHECK-NEXT: -3
    outputBuffer[6] = c % d; // CHECK-NEXT: -1
    int i = -2147483647 - 1;
    int j = -1;
    outputBuffer[7] = int64_t(i / j); // CHECK-NEXT: -2147483648
    outputBuffer[8] = int64_t(i % j); // CHECK-NEXT: 0
    int64_t e = 7ll;
    outputBuffer[9] = e / b;  // CHECK-NEXT: -7
    outputBuffer[10] = e % b; // CHECK-NEXT: 0

    // Front-end folding of an explicit operator call, which is not rewritten to a
    // builtin-operator expression.
    int f0[(operator/(kMin, -1ll) == kMin) ? 1 : 2];
    int f1[(operator%(kMin, -1ll) == 0ll) ? 1 : 2];
    int f2[(operator/(0x8000000000000000ull, 0xFFFFFFFFFFFFFFFFull) == 0ull) ? 1 : 2];
    int f3[(operator%(0x8000000000000000ull, 0xFFFFFFFFFFFFFFFFull) == kTop) ? 1 : 2];
    int f4[(operator/(-7ll, 2ll) == -3ll && operator%(-7ll, 2ll) == -1ll) ? 1 : 2];
    outputBuffer[11] = f0.getCount(); // CHECK-NEXT: 1
    outputBuffer[12] = f1.getCount(); // CHECK-NEXT: 1
    outputBuffer[13] = f2.getCount(); // CHECK-NEXT: 1
    outputBuffer[14] = f3.getCount(); // CHECK-NEXT: 1
    outputBuffer[15] = f4.getCount(); // CHECK-NEXT: 1

    // Front-end folding of the builtin-operator expression.
    int b0[(kMin / -1ll == kMin) ? 1 : 2];
    int b1[(kMin % -1ll == 0ll) ? 1 : 2];
    int b2[(kTop / kMax == 0ull) ? 1 : 2];
    int b3[(kTop % kMax == kTop) ? 1 : 2];
    int b4[(kMax / 2ull == 0x7FFFFFFFFFFFFFFFull) ? 1 : 2];
    outputBuffer[16] = b0.getCount(); // CHECK-NEXT: 1
    outputBuffer[17] = b1.getCount(); // CHECK-NEXT: 1
    outputBuffer[18] = b2.getCount(); // CHECK-NEXT: 1
    outputBuffer[19] = b3.getCount(); // CHECK-NEXT: 1
    outputBuffer[20] = b4.getCount(); // CHECK-NEXT: 1

    int b5[(~0u / 1u == ~0u && operator/(~0u, 1u) == ~0u) ? 1 : 2];
    int b6[((~0x7FFFFFFFu) % 0xFFFFFFFFu == ~0x7FFFFFFFu) ? 1 : 2];
    outputBuffer[21] = b5.getCount(); // CHECK-NEXT: 1
    outputBuffer[22] = b6.getCount(); // CHECK-NEXT: 1

    // Generic value parameters, folded by SCCP after specialization.
    outputBuffer[23] = sizeFromSignedGeneric<kMin>();   // CHECK-NEXT: 1
    outputBuffer[24] = sizeFromUnsignedGeneric<kTop>(); // CHECK-NEXT: 1
}
