//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// Regression test for a type-flow specialization bug.
//
// `shapes[0]` is a dynamic existential. `(shapes[0] as Square).value` lowers to an
// `as`-cast: a `makeOptionalValue` on the true branch and a `makeOptionalNone` on the
// false branch, merged at a phi whose declared type is `Optional<Square>`.
//
// `Square` is *non-concrete* in the type-flow pass because it contains an interface-typed
// member (`extra`). On that path `analyzeMakeOptionalValue` produced no payload info
// (null), while `analyzeMakeOptionalNone` produced an `OptionalNoneType` refinement.
// `unionPropagationInfo` then short-circuited on the null info and adopted the
// `OptionalNoneType`, retyping the whole phi to a payload-less none. Downstream, the
// unwrapped `.value` could no longer be projected, so the method receiver (`this`, passed
// by pointer because `Square` is large) was bound to the whole `Optional<Square>` wrapper
// instead of the value — emitting target code that referenced a non-existent overload.
//
// With the fix the phi keeps its declared `Optional<Square>` type, `.value` projects
// correctly, and `area()`/`perimeter()` receive a proper `Square*`.

[anyValueSize(16)]
interface IExtra
{
    float k();
}

struct ExtraImpl : IExtra
{
    float v;
    float k() { return v; }
}

// A second, never-instantiated `IExtra` implementation. The witness header in the buffer
// always selects `ExtraImpl` (seq id 1), so this is never used at runtime — but registering a
// second conformance keeps `IExtra` dynamic, so `extra.k()` dispatches through the runtime
// witness rather than being devirtualized. That makes the inner existential's header load-bearing.
struct ExtraImpl2 : IExtra
{
    float w;
    float k() { return w + 1.0f; }
}

[anyValueSize(64)]
interface IShape
{
    float area();
    float perimeter();
}

struct Square : IShape
{
    // Padded so the struct is large enough that methods take `this` by pointer.
    float side;
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
    float g;
    // Interface-typed member: makes `Square` non-concrete during type-flow specialization,
    // which is what exercises the buggy merge path.
    IExtra extra;

    float area() { return side * side + a + b + c + d + e + f + g; }
    float perimeter() { return side * 4.0f; }
}

// A second, never-instantiated `IShape` implementation, for the same reason: it keeps `IShape`
// dynamic so `shapes[0] as Square` is a genuine runtime type test (driven by the outer witness
// header) rather than a statically-resolved one.
struct Triangle : IShape
{
    float base;
    float height;
    float area() { return 0.5f * base * height; }
    float perimeter() { return base * 3.0f; }
}

//TEST_INPUT: type_conformance Square:IShape = 1
//TEST_INPUT: type_conformance Triangle:IShape = 2
//TEST_INPUT: type_conformance ExtraImpl:IExtra = 1
//TEST_INPUT: type_conformance ExtraImpl2:IExtra = 2

// One existential box for `shapes[0]`:
//   header: RTTI handle [0 0], witness handle [1 0]  (Square, seq id 1)
//   payload (Square): side=3.0 (0x40400000=1077936128), a..g = 0   (8 words)
//                     extra (IExtra box): header [0 0] [1 0], payload v=0,0,0,0  (8 words)
//TEST_INPUT: set shapes = ubuffer(data=[0 0 1 0 1077936128 0 0 0 0 0 0 0 0 0 2 0 0 0 0 0], stride=4)
StructuredBuffer<IShape> shapes;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let sq = (shapes[0] as Square).value;
    outputBuffer[tid.x] = sq.area() + sq.perimeter() + sq.extra.k();
    // area = 3*3 = 9, perimeter = 3*4 = 12
    // CHECK: 22.0
}
