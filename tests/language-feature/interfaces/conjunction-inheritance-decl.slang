// conjunction-inheritance-decl.slang

// Test that `&` in an inheritance declaration is flattened
// into individual conformances (e.g. `struct Foo : IBar & IBaz`).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IFoo
{
    int getFoo();
}

interface IBar
{
    int getBar();
}

// Use `&` directly in the inheritance clause instead of separate conformances.
struct MyStruct : IFoo & IBar
{
    int x;
    int y;

    int getFoo() { return x; }
    int getBar() { return y; }
}

int combine<T : IFoo & IBar>(T val)
{
    return val.getFoo() * 10 + val.getBar();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// CHECK: 12
// CHECK: 34
// CHECK: 56
// CHECK: 78

[numthreads(4, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    MyStruct s;
    s.x = tid.x * 2 + 1;
    s.y = tid.x * 2 + 2;
    outputBuffer[tid.x] = combine(s);
}
