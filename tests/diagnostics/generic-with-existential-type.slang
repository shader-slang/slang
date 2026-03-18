//TEST:SIMPLE(filecheck=CHECK): -target spirv
// Regression test for https://github.com/shader-slang/slang/issues/8635
// Passing a dynamic dispatch object (existential type) to a generic function
// should compile successfully (via dynamic dispatch) rather than producing
// an "unexpected IR opcode" internal compiler error.
//
// CHECK: OpEntryPoint

interface IFoo
{
    int getValue();
}

export struct FooImpl : IFoo
{
    int value;
    int getValue() { return value; }
}

export struct FooImpl2 : IFoo
{
    int value;
    int getValue() { return value * 2; }
}

void genericFunc<T : IFoo>(T v)
{
    int x = v.getValue();
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo v = createDynamicObject<IFoo>(0, 0);
    genericFunc(v); // compiles via dynamic dispatch in legacy mode (no #lang slang 2025)
}
