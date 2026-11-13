// Calls to the scalar integer `max`, `min`, `abs` and `clamp` built-ins on compile-time constant
// arguments are compile-time constants, so they can size an array (shader-slang/slang#13973).
//TEST:INTERPRET(filecheck=CHECK):

static const int A = 8;
static const int B = 4;
static const int N = max(A, B) + 2;

// With a generic value parameter the call stays symbolic until the generic is specialized.
int genericSizes<let K : int>()
{
    int arrMax[max(K, 4)];
    int arrMin[min(K, 4)];
    int arrClamp[clamp(K, 2, 6)];
    int arrAbs[abs(K - 10)];
    return arrMax.getCount() * 1000 + arrMin.getCount() * 100 + arrClamp.getCount() * 10 +
           arrAbs.getCount();
}

void main()
{
    int arrMaxPlus[max(A, B) + 2];
    int arrMaxLit[max(8, 4)];
    int arrMin[min(A, B)];
    int arrAbs[abs(-A)];
    int arrClamp[clamp(A, 0, 4)];
    int arrUInt[max(3u, 2u)];
    int arrStatic[N];
    // CHECK: 10 8 4 8 4 3 10
    printf(
        "%d %d %d %d %d %d %d\n",
        arrMaxPlus.getCount(),
        arrMaxLit.getCount(),
        arrMin.getCount(),
        arrAbs.getCount(),
        arrClamp.getCount(),
        arrUInt.getCount(),
        arrStatic.getCount());

    // Signed operands compare as signed.
    int arrSignedMin[min(-3, 2) == -3 ? 2 : 9];
    int arrSignedMax[max(-3, 2) == 2 ? 3 : 9];
    int arrSignedClamp[clamp(-5, -4, -1) == -4 ? 4 : 9];
    // CHECK: 2 3 4
    printf("%d %d %d\n", arrSignedMin.getCount(), arrSignedMax.getCount(), arrSignedClamp.getCount());

    // Unsigned operands compare as unsigned: `0u - 1u` is 0xFFFFFFFF, and the 64-bit all-ones
    // value is the largest `uint64_t`.
    int arrUIntWrap[min(0u - 1u, 5u)];
    int arrUInt64[max(0xFFFFFFFFFFFFFFFFull, 1ull) == 0xFFFFFFFFFFFFFFFFull ? 3 : 7];
    // `abs(INT_MIN)` wraps to `INT_MIN`, as it does on the targets.
    int arrAbsMin[abs(-2147483647 - 1) == -2147483647 - 1 ? 2 : 9];
    // CHECK: 5 3 2
    printf("%d %d %d\n", arrUIntWrap.getCount(), arrUInt64.getCount(), arrAbsMin.getCount());

    // genericSizes<7>: max(7,4)=7, min(7,4)=4, clamp(7,2,6)=6, abs(7-10)=3.
    // CHECK: 7463
    printf("%d\n", genericSizes<7>());
}
