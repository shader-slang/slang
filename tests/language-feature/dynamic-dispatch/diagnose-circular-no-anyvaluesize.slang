// Circular interface conformance without [anyValueSize] should produce a clear
// error, not an internal compiler error.
//
// When no [anyValueSize] is specified, the compiler infers the AnyValue size
// from conforming types. A circular dependency (struct conforms to an interface
// and contains a field of that interface type) means there is no base case for
// size inference.

// Disabled: the new IR translate pass from the autodiff refactor (#9808) crashes
// on circular dyn interfaces without anyValueSize before the diagnostic pass runs.
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv

dyn interface IFoo
{
    float getValue();
}

// CHECK: error[E41003]
// CHECK: BadImpl
// CHECK: circular reference through interface
// CHECK: IFoo
struct BadImpl : IFoo
{
    IFoo child;
    float getValue() { return child.getValue(); }
}

IFoo factory(uint id)
{
    return BadImpl();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo obj = factory(0);
    outputBuffer[0] = obj.getValue();
}
