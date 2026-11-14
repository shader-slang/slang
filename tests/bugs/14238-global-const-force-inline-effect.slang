// A module-scope `static const` whose initializer calls a helper with an effect, here an atomic
// add, is evaluated in the function that uses it. Global value legalization copies the initializer
// there and removes the original at module scope. CUDA, C++, GLSL, Metal and WGSL cannot initialize
// a global with a call, and HLSL would run the effect again when it initializes the global.
//
// `Holder.kPair` is a struct built by one call. `Holder.arr` is an array whose elements are calls,
// so each element call is an operand of the array. Both are read once, so each call runs once.
// `Unused.kUnused` is never read, so its call never runs.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type

//TEST_INPUT: set counters = out ubuffer(data=[5 10], stride=4)
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

[ForceInline]
int bump()
{
    return counters[1].add(1);
}

struct Holder
{
    static const Pair kPair = snapshot();
    static const int arr[2] = { bump(), bump() };
}

struct Unused
{
    static const int kUnused = bump();
}

// GLSL-NOT: atomicAdd
// GLSL: void main(
// GLSL: atomicAdd
// CUDA-NOT: atomicAdd
// CUDA: void computeMain(
// CUDA: atomicAdd
// HLSL-NOT: InterlockedAdd
// HLSL: void computeMain(
// HLSL: InterlockedAdd
// METAL-NOT: atomic_fetch_add
// METAL: void computeMain(
// METAL: atomic_fetch_add
// WGSL-NOT: atomicAdd
// WGSL: fn computeMain(
// WGSL: atomicAdd

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = Holder.kPair.a;
    outputBuffer[tid.x + 1] = Holder.arr[tid.x];
    // The first counter goes from 5 to 6 and the second from 10 to 12; the results are the old
    // values of the first atomic add of each.
    // BUF: type: int32_t
    // BUF-NEXT: 6
    // BUF-NEXT: 12
    // BUF: type: int32_t
    // BUF-NEXT: 5
    // BUF-NEXT: 10
}
