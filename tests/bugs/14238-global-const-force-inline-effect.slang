// A `[ForceInline]` helper whose body performs an atomic add is not inlined into a module-scope
// `static const` initializer, because the global scope cannot perform the effect. The call stays a
// call, and global value legalization moves it into the function that uses the constant, where it
// is inlined, so the effect runs once.
//
// A helper that divides by a value that does not fold, without using the quotient, is inlined.
// The division is then an unused module-scope value with no effect of its own, and is removed.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type

//TEST_INPUT: set counters = out ubuffer(data=[5], stride=4)
RWStructuredBuffer<Atomic<int>> counters;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct Pair
{
    int a;
    int b;
}

[ForceInline]
Pair snapshot()
{
    Pair p = { counters[0].add(1), 2 };
    return p;
}

[__readNone]
[ForceInline]
int incrementIgnoringQuotient(int a, int b)
{
    int quotient = a / b;
    return a + 1;
}

struct Holder
{
    static const Pair kPair = snapshot();
    static const int kIncremented = incrementIgnoringQuotient(10, max(4, 1));
}

// GLSL-NOT: atomicAdd
// GLSL-NOT: max(
// GLSL: void main(
// GLSL: atomicAdd
// GLSL-NOT: max(

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = Holder.kPair.a;
    outputBuffer[tid.x + 1] = Holder.kIncremented;
    // The atomic add runs once: the counter goes from 5 to 6, and the result is the old value.
    // BUF: type: int32_t
    // BUF-NEXT: 6
    // BUF: type: int32_t
    // BUF-NEXT: 5
    // BUF-NEXT: 11
}
