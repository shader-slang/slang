// Locals of structs that point to each other compile and link through their pointers
// (shader-slang/slang#13404).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type

struct A
{
    float x;
    B* b;
};

struct B
{
    float y;
    A* a;
};

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    A a;
    B b;
    a.x = 1.0;
    b.y = 2.0;
    a.b = &b;
    b.a = &a;

    // CHECK: 1.0
    // CHECK-NEXT: 2.0
    // CHECK-NEXT: 1.0
    outputBuffer[0] = a.x;
    outputBuffer[1] = a.b->y;
    outputBuffer[2] = a.b->a->x;
}
