//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// A `__constraint` is only meaningful as a requirement of an interface. Using it in
// any other declaration context (here, a `struct` body) is rejected centrally by the
// parser's `isDeclAllowed` nesting check, like any other misplaced declaration.

interface IFoo
{
    int f();
}

struct Bad
{
    __constraint int : IFoo;
    /*diag:
                     ^ declaration not allowed
                     ^ GenericTypeConstraintDecl is not allowed here.
    */
}
