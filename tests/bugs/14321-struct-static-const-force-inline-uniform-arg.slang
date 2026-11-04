// A force-inlined helper given a uniform argument in a struct `static const` initializer
// compiles with debug information. The helper is inlined outside of any function, and its
// debug information is not, because there is no debug scope there. A debug value left there
// would keep a use of the uniform outside of any function, which the C++, CPU and Metal
// targets cannot lower. The generic struct covers a helper that is inlined early, while the
// initializer is still in the body of the generic.

//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none -g3
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none -g3
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type -Xslang -g
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type -Xslang -g

//TEST_INPUT: set divisor = 4
uniform int divisor;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[__readNone]
[ForceInline]
int ceilDiv(int a, int b)
{
    return (a + b - 1) / b;
}

[__readNone]
[ForceInline]
int addOne(int a)
{
    return a + 1;
}

[__readNone]
[__unsafeForceInlineEarly]
int addTwo(int a)
{
    return a + 2;
}

struct Holder
{
    static const int kQuotient = ceilDiv(10, divisor);
    static const int kSum = addOne(divisor);
}

struct GenericHolder<let N : int>
{
    static const int kSum = addTwo(divisor + N);
}

// CPP: void _computeMain(
// METAL: void computeMain(

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = Holder.kQuotient;
    outputBuffer[tid.x + 1] = Holder.kSum;
    outputBuffer[tid.x + 2] = GenericHolder<3>.kSum;
    // BUF: type: int32_t
    // BUF-NEXT: 3
    // BUF-NEXT: 5
    // BUF-NEXT: 9
}
