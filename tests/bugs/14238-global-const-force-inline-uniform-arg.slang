// A `[ForceInline]` helper given a uniform argument in a module-scope `static const` initializer
// compiles with debug information. The helper is inlined at the global scope, and its debug
// information is not, because the global scope has no debug scope. A debug value left there
// would keep a use of the uniform outside of any function, which the C++ and Metal targets
// cannot evaluate.

//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none -g
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type

//TEST_INPUT: set divisor = 4
uniform int divisor;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
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

struct Holder
{
    static const int kQuotient = ceilDiv(10, divisor);
    static const int kSum = addOne(divisor);
}

// CPP: void _computeMain(
// METAL: void computeMain(

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = Holder.kQuotient;
    outputBuffer[tid.x + 1] = Holder.kSum;
    // BUF: type: int32_t
    // BUF-NEXT: 3
    // BUF-NEXT: 5
}
