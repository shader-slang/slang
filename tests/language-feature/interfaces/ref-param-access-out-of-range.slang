// An `Access` argument on `RefParam` other than `Read` or `Immutable` leaves the
// reference read-write, so the type is diagnosed rather than causing an internal error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct S
{
    RefParam<float, (Access)7> p;
//CHECK:                       ^ reference type cannot be used as struct field
//CHECK:                       ^ 'ref float' cannot be used as a struct field; reference types can only be used for function parameters and return values.
}
