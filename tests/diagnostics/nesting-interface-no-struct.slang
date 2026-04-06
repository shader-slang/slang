//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Struct types are not allowed inside an interface (caught by parser).

interface IFoo
{
    struct Inner {}
//CHECK:   ^^^^^ declaration not allowed here
//CHECK:   ^^^^^ struct is not allowed here.
}
