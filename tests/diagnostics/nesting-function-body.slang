//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Declarations inside function bodies should be validated against the
// nesting rules (getParentDecl unwraps ScopeDecl to reach FunctionDeclBase).

// Valid declarations inside a function body: no diagnostics expected.
void validFunction()
{
    int x = 0;
    struct LocalStruct { int a; };
    typealias MyInt = int;
}

// Invalid: interface is not allowed inside a function body.
void invalidFunction()
{
    interface IBad {}
//CHECK:      ^^^^ declaration not allowed in this context
//CHECK:      ^^^^ interface declaration is not allowed inside function.
}
