// Runtime regression for https://github.com/shader-slang/slang/issues/11917.
//
// Companion to tagged-union-lowering-gated.slang (which is a compile/emission guard). This test
// actually EXECUTES the dynamic dispatch that `lowerTaggedUnionTypes` lowers: an `IShape`-typed
// value whose concrete type (`Square` or `Rect`) is chosen at runtime is a tagged union (a tag
// selecting the witness table, plus a payload), and calling `area()` through it must dispatch to
// the correct implementation. If the pass's gate wrongly skipped the lowering, dispatch would break
// and the computed areas would be wrong — so the checked values below are the runtime guard.
//
// Runs on the CPU target so it executes in CI without a GPU; also covered on the GPU targets.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type

[anyValueSize(16)]
interface IShape
{
    int area();
}

struct Square : IShape
{
    int side;
    int area() { return side * side; }
}

struct Rect : IShape
{
    int w;
    int h;
    int area() { return w * h; }
}

//TEST_INPUT: type_conformance Square:IShape = 0
//TEST_INPUT: type_conformance Rect:IShape = 1

// Passing through an `IShape`-typed local defeats static specialization at the call site, so the
// compiler must build a dynamic (tagged-union) representation that can hold either concrete type.
int areaOf(IShape s)
{
    return s.area();
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    IShape s;
    // Even threads take a Square, odd threads take a Rect: both concrete types flow through the
    // same `IShape` local, forcing the tagged-union representation and runtime dispatch.
    if ((i & 1) == 0)
    {
        Square sq = { 3 };   // area = 9
        s = sq;
    }
    else
    {
        Rect r = { 2, 4 };   // area = 8
        s = r;
    }
    outputBuffer[i] = areaOf(s);
}

// Thread 0: Square area 9; thread 1: Rect area 8; thread 2: Square 9; thread 3: Rect 8.
// CHECK: 9
// CHECK-NEXT: 8
// CHECK-NEXT: 9
// CHECK-NEXT: 8
