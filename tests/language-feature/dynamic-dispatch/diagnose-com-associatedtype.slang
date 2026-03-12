// COM interfaces use vtable-based dispatch with a fixed binary layout,
// which is incompatible with associated types that would require
// compile-time type resolution to determine the vtable layout.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

[COM("00000000-0000-0000-0000-000000000001")]
interface ICOMWithAssocType
{
    associatedtype Element;
//CHECK:           ^^^^^^^ E31122
//CHECK:           ^^^^^^^ associatedtype not allowed in a [COM] interface
    float getValue();
}
