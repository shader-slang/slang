//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Declaration kinds NOT allowed inside a struct (caught by parser).

struct S
{
    interface IBad {}
//CHECK:      ^^^^ declaration not allowed here
//CHECK:      ^^^^ interface is not allowed here.

    extension S {}
//CHECK:      ^ declaration not allowed here
//CHECK:      ^ extension is not allowed here.

    namespace BadNS {}
//CHECK:      ^^^^^ declaration not allowed here
//CHECK:      ^^^^^ namespace is not allowed here.
}
