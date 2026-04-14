//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

interface Base
{
    static int getValue();
//CHECK:       ^^^^^^^^ see interface requirement declaration of 'getValue'
}

struct Impl : Base
{
    // This is not static and not allowed to implement the interface's static method.
    int getValue() { return 5; }
//CHECK:^^^^^^^^ interface requirement mismatch
//CHECK:^^^^^^^^ member 'getValue' does not match interface requirement.
}

int callGet<Foo : Base>()
{
    return Foo::getValue();
}

RWStructuredBuffer<int> result;

[numthreads(1, 1, 1)]
void computeMain()
{
    result[0] = callGet<Impl>();
}
