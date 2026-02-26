// COM interfaces use a vtable-based dispatch mechanism that is fundamentally
// different from regular Slang interface dispatch (tagged union / AnyValue).
// Mixing the two dispatch models within an inheritance chain would produce
// inconsistent calling conventions, so any interface inheriting from a COM
// interface must itself be marked as COM.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

[COM("00000000-0000-0000-0000-000000000001")]
interface ICOMBase
{
    float getValue();
}

// CHECK: ([[# @LINE+1]]): error 31124
interface INonCOM : ICOMBase
{
    float compute();
}
