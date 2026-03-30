//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Declarations inside function bodies should be validated against the
// nesting rules (getParentContainerForNesting unwraps ScopeDecl to reach
// FunctionDeclBase).  These valid declarations must not trigger false
// positives from the Function container category.

void validFunction()
{
    int x = 0;
    struct LocalStruct { int a; };
    typealias MyInt = int;
}
