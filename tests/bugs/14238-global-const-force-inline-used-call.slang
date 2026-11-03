// A `[ForceInline]` helper that divides the result of a call that is not inlined is inlined into a
// module-scope `static const` initializer. The callee has no side effects, so the call is a value,
// and the constant is evaluated where it is used.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type -Xslang -g0
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type -Xslang -g0

// The execution lanes use -g0. With debug information, `triple` is not inferred to be free of side
// effects, so the helper stays a call, which a CUDA or C++ global initializer cannot evaluate. A
// `static const` that calls `triple` directly fails the same way, so the limit is not in inlining.

[noinline]
int triple(int x)
{
    return x * 3;
}

[ForceInline]
int tripleThenDiv(int a, int b)
{
    return triple(a) / b;
}

struct Holder
{
    static const int value = tripleThenDiv(4, 2);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// CUDA-NOT: tripleThenDiv
// CUDA: void computeMain(
// CUDA: triple_0(int(4)) / int(2)
// CPP-NOT: tripleThenDiv
// CPP: void _computeMain(
// CPP: triple_0(int(4)) / int(2)
// GLSL-NOT: tripleThenDiv
// GLSL: void main(
// GLSL: triple_0(4) / 2

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = Holder.value;
    // BUF: 6
}
