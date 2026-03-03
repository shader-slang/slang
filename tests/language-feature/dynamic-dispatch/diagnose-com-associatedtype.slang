// COM interfaces use vtable-based dispatch with a fixed binary layout,
// which is incompatible with associated types that would require
// compile-time type resolution to determine the vtable layout.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

[COM("00000000-0000-0000-0000-000000000001")]
interface ICOMWithAssocType
{
    // CHECK: ([[# @LINE+1]]): error 31122
    associatedtype Element;
    float getValue();
}
