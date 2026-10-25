// `-fp-mode precise` must turn off the `metal` compiler's default fast math when Slang
// produces a metallib. Under fast math the compiler assumes no NaN or Inf, so it folds
// `x != x` to false and `x * 0.0 == 0.0` to true.

//TEST:SIMPLE(filecheck=PRECISE): -target metallib-asm -entry computeMain -stage compute -fp-mode precise
//TEST:SIMPLE(filecheck=FAST): -target metallib-asm -entry computeMain -stage compute -fp-mode fast

// With inputs (0, 0, 1), x is NaN and inf is +Inf at run time. Expected output: 1, 1, 1, 0.
StructuredBuffer<uint> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float zero = asfloat(inputBuffer[0]);
    float one = asfloat(inputBuffer[2]);
    float x = zero / asfloat(inputBuffer[1]); // NaN
    float inf = one / zero;                   // +Inf

    outputBuffer[0] = isnan(x) ? 1 : 0;
    outputBuffer[1] = (x != x) ? 1 : 0;
    outputBuffer[2] = isinf(inf) ? 1 : 0;
    outputBuffer[3] = (x * 0.0f == 0.0f) ? 1 : 0;
}

// PRECISE: define {{.*}}@computeMain
// PRECISE-NOT: fast
// PRECISE: fdiv float
// PRECISE-NOT: fast
// PRECISE: ret void

// FAST: fdiv fast float
