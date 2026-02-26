// COM interfaces rely on vtable-based dispatch through pointer indirection,
// which requires reference semantics. Slang structs are value types that do
// not support the pointer-based object model COM requires, so structs cannot
// implement COM interfaces.

// Disabled: error 31124 does not fire due to a bug in checkConformance()
// that checks the parent type instead of the child type. Enable when fixed.
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv

[COM("00000000-0000-0000-0000-000000000001")]
interface ICOMService
{
    float query();
}

// CHECK: ([[# @LINE+1]]): error 31124
struct BadImpl : ICOMService
{
    float v;
    float query() { return v; }
}
