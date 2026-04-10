//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Interface inside a struct is not allowed (caught by parser).
// Interface inside another interface is also not allowed.

struct S
{
    interface IBadInStruct {}
//CHECK:      ^^^^^^^^^^^^ declaration not allowed here
//CHECK:      ^^^^^^^^^^^^ interface is not allowed here.
}

interface IOuter
{
    interface IBadInInterface {}
//CHECK:      ^^^^^^^^^^^^^^^ declaration not allowed here
//CHECK:      ^^^^^^^^^^^^^^^ interface is not allowed here.
}
