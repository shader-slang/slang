// COM interfaces rely on vtable-based dispatch through pointer indirection,
// which requires reference semantics. Slang structs are value types that do
// not support the pointer-based object model COM requires, so structs cannot
// implement COM interfaces.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

[COM("00000000-0000-0000-0000-000000000001")]
interface ICOMService
{
    float query();
}

struct BadImpl : ICOMService
//CHECK:         ^^^^^^^^^^^ E31124
//CHECK:         ^^^^^^^^^^^ a struct type cannot implement a [COM] interface
{
    float v;
    float query() { return v; }
}
