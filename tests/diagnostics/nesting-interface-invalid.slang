//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Non-static member variable is not allowed inside an interface.
// The parser allows VarDecl in InterfaceDecl, but the semantic nesting
// validation rejects non-static fields.

interface IBad
{
    int bad_field;
//CHECK:^^^^^^^^^ declaration not allowed in this context
//CHECK:^^^^^^^^^ non-static member variable declaration is not allowed inside interface.
}
