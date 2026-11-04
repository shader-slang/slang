// A module-scope `static const` built by calls to helpers that use integer `/` or `%` is evaluated
// in each function that uses it. Global value legalization copies a value that depends on a call
// into those functions, so no target sees a global initializer that calls a function, which CUDA,
// GLSL and C++ reject. The language does not promise a compile-time constant here; a value whose
// arguments are literals may still fold once it is in the function.
//
// `kDiv` and `kRem` divide by a parameter, `Holder.arr` uses a helper without `[__readNone]`,
// `Holder.plain` uses a helper that is not inlined, and `kScalar` is a scalar. `kLiteralDivisor`
// divides by a literal. `kSafe` and `kHandWritten` divide by `max(4, 1)`. `Holder.arr` and
// `Holder.plain` are the cases whose calls are left at module scope unless legalization removes the
// operands it copied; the other constants are controls.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute
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

int ceilDivPlain(int a, int b)
{
    return (a + b - 1) / b;
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
    static const int plain[2] = { ceilDivPlain(10, 4), ceilDivPlain(7, 2) };
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// No global is initialized by a call: before the entry point there is no call with constant
// arguments, while helper definitions may appear. Folded literals may stay global. The values of
// `Holder.plain`, whose helper is not inlined, are computed in the entry point.
// CUDA-NOT: {{[A-Za-z]+_0\(int\(}}
// CUDA: void computeMain(
// CUDA: plain_0 = { {
// CPP-NOT: {{[A-Za-z]+_0\(int\(}}
// CPP: void _computeMain(
// CPP: plain_0 = { {
// GLSL-NOT: {{[A-Za-z]+_0\([0-9]}}
// GLSL: void main(
// GLSL: plain_0[2] =
// SPV: OpEntryPoint

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // A dynamic index keeps each array an array rather than one value per read.
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
    outputBuffer[12] = Holder.plain[i];
    outputBuffer[13] = Holder.plain[i + 1];
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
    // BUF-NEXT: 3
    // BUF-NEXT: 4
}
