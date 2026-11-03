// A module-scope `static const` built by `[ForceInline]` helpers that use integer `/` or `%`
// folds to constants. An integer division by a parameter might trap, so it counts as having a
// side effect inside a function, but at module scope it is a value evaluated at its uses, so the
// helper body is still valid at the global scope. CUDA, GLSL and C++ reject a global initializer
// that calls a function. A division whose divisor does not fold, written directly or through a
// helper, is evaluated where the constant is used.
//
// `kDiv` and `kRem` divide by a parameter, `Holder.arr` uses a helper without `[__readNone]`, and
// `kScalar` is a scalar. `kLiteralDivisor` divides by a literal, which already had no side effect.
// `kSafe` and `kHandWritten` divide by `max(4, 1)`, a call that constant propagation does not fold.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type

[__readNone]
[ForceInline]
int ceilDiv(int a, int b)
{
    return (a + b - 1) / b;
}

[__readNone]
[ForceInline]
uint wrap(uint a, uint n)
{
    return a % n;
}

[ForceInline]
int ceilDivNotReadNone(int a, int b)
{
    return (a + b - 1) / b;
}

[__readNone]
[ForceInline]
int halve(int a)
{
    return a / 2;
}

[__readNone]
[ForceInline]
int ceilDivSafe(int a, int b)
{
    return (a + b - 1) / max(b, 1);
}

static const int kDiv[2] = { ceilDiv(10, 4), ceilDiv(7, 2) };
static const uint kRem[3] = { wrap(7, 3), wrap(8, 3), wrap(9, 4) };
static const int kScalar = ceilDiv(10, 4);
static const int kLiteralDivisor[2] = { halve(10), halve(7) };
static const int kSafe = ceilDivSafe(10, 4);
static const int kHandWritten = 10 / max(4, 1);

struct Holder
{
    static const int arr[2] = { ceilDivNotReadNone(10, 4), ceilDivNotReadNone(7, 2) };
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// Each array is initialized with literals, `kScalar` is the literal 3 at its use, `kSafe` and
// `kHandWritten` are divisions at their uses, and no call to a helper remains.
// CUDA-NOT: {{ceilDiv|wrap_|halve_|max}}
// CUDA: kLiteralDivisor_0 = { {
// CUDA-NEXT: int(5), int(3)
// CUDA: arr_0 = { {
// CUDA-NEXT: int(3), int(4)
// CUDA: kRem_0 = { {
// CUDA-NEXT: 1U, 2U, 1U
// CUDA: kDiv_0 = { {
// CUDA-NEXT: int(3), int(4)
// CUDA-NOT: {{ceilDiv|wrap_|halve_|max}}
// CUDA: void computeMain(
// CUDA-NOT: {{ceilDiv|wrap_|halve_}}
// CUDA: = int(3);
// CUDA: = int(13) / ({{.*}}max
// CUDA: = int(10) / ({{.*}}max
// CUDA-NOT: {{ceilDiv|wrap_|halve_}}
// CPP-NOT: {{ceilDiv|wrap_|halve_|max}}
// CPP: kLiteralDivisor_0 = { { int(5), int(3) } };
// CPP: arr_0 = { { int(3), int(4) } };
// CPP: kRem_0 = { { 1U, 2U, 1U } };
// CPP: kDiv_0 = { { int(3), int(4) } };
// CPP-NOT: {{ceilDiv|wrap_|halve_|max}}
// CPP: void _computeMain(
// CPP-NOT: {{ceilDiv|wrap_|halve_}}
// CPP: = int(3);
// CPP: = int(13) / ({{.*}}max
// CPP: = int(10) / ({{.*}}max
// CPP-NOT: {{ceilDiv|wrap_|halve_}}
// GLSL-NOT: {{ceilDiv|wrap_|halve_|max\(}}
// GLSL: kLiteralDivisor_0[2] = int[](5, 3);
// GLSL: arr_0[2] = int[](3, 4);
// GLSL: kRem_0[3] = uint[](1U, 2U, 1U);
// GLSL: kDiv_0[2] = int[](3, 4);
// GLSL-NOT: {{ceilDiv|wrap_|halve_|max\(}}
// GLSL: void main(
// GLSL-NOT: {{ceilDiv|wrap_|halve_}}
// GLSL: = 3;
// GLSL: = 13 / max(
// GLSL: = 10 / max(
// GLSL-NOT: {{ceilDiv|wrap_|halve_}}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // A dynamic index keeps each array a global constant.
    uint i = tid.x;
    outputBuffer[0] = kDiv[i];
    outputBuffer[1] = kDiv[i + 1];
    outputBuffer[2] = int(kRem[i]);
    outputBuffer[3] = int(kRem[i + 1]);
    outputBuffer[4] = int(kRem[i + 2]);
    outputBuffer[5] = Holder.arr[i];
    outputBuffer[6] = Holder.arr[i + 1];
    outputBuffer[7] = kScalar;
    outputBuffer[8] = kLiteralDivisor[i];
    outputBuffer[9] = kLiteralDivisor[i + 1];
    outputBuffer[10] = kSafe;
    outputBuffer[11] = kHandWritten;
    // BUF: type: int32_t
    // BUF-NEXT: 3
    // BUF-NEXT: 4
    // BUF-NEXT: 1
    // BUF-NEXT: 2
    // BUF-NEXT: 1
    // BUF-NEXT: 3
    // BUF-NEXT: 4
    // BUF-NEXT: 3
    // BUF-NEXT: 5
    // BUF-NEXT: 3
    // BUF-NEXT: 3
    // BUF-NEXT: 2
}
