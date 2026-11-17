// A module-scope `static const` built by a `[ForceInline]` helper that divides the result of a
// call that is not inlined is evaluated where the constant is used, with or without debug
// information. The execution lanes run with debug information, which render-test enables by
// default.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type

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
