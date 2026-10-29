//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute -line-directive-mode none -Wno-30816
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none -Wno-30816
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type -Xslang -Wno-30816
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type -Xslang -Wno-30816

// A field of an `__extern_cpp` struct keeps its source name in C++ and CUDA
// output (https://github.com/shader-slang/slang/issues/14028). Lowering names
// the field that holds an inherited struct `base`, so a source field `base` of
// a derived struct shares that name. Both get numbered names that no other
// member uses, so the emitted struct has no duplicate member. A source field
// that looks like a numbered name (`base_0`) keeps its own name, and so does
// a field marked `__extern_cpp` itself (`base_1`).

//CHECK: struct Derived
//CHECK-NEXT: {
//CHECK-NEXT: Base base_2;
//CHECK-NEXT: float base_3;
//CHECK-NEXT: float base_0;
//CHECK-NEXT: float base_1;
//CHECK-NEXT: float y;
//CHECK-NEXT: };

__extern_cpp struct Base
{
    float x;
}

__extern_cpp struct Derived : Base
{
    float base;
    float base_0;
    __extern_cpp float base_1;
    float y;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Derived d;
    d.x = 1.0;
    d.base = 2.0;
    d.base_0 = 8.0;
    d.base_1 = 16.0;
    d.y = 4.0;
    // BUF: 31
    outputBuffer[0] = d.x + d.base + d.base_0 + d.base_1 + d.y;
}
