//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Enum types are not allowed inside an interface (caught by parser).

interface IFoo
{
    enum E { A, B }
//CHECK: ^ declaration not allowed here
//CHECK: ^ enum is not allowed here.
}
