//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Regression test for treating nominal structs as structural types during type-flow. Recursing
// into `Square.extra` made `Optional<Square>` appear non-concrete, so the none branch retyped the
// downcast's merge phi to `OptionalNoneType`. Keeping structs nominal preserves the declared
// `Optional<Square>` and its payload. The `extra.k()` call also checks existential field
// extraction.

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
    IExtra extra;

    float area() { return side * side + a + b + c + d + e + f + g; }
    float perimeter() { return side * 4.0f; }
}

// Keep `IShape` dynamic so the downcast performs a runtime type test.
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

//TEST_INPUT: set shapes = ubuffer(data=[0 0 1 0 1077936128 0 0 0 0 0 0 0 0 0 2 0 0 0 0 0], stride=4)
StructuredBuffer<IShape> shapes;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let sq = (shapes[0] as Square).value;
    outputBuffer[tid.x] = sq.area() + sq.perimeter() + sq.extra.k();
    // area = 9, perimeter = 12, extra.k() = 1
    // CHECK: 22.0
}
