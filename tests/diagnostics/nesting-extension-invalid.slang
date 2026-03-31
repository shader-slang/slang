//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Non-static member variable is not allowed inside an extension (issue #9599).
// The parser allows VarDecl in ExtensionDecl, but the semantic nesting
// validation rejects non-static fields.

struct ForExt { int a; }

extension ForExt
{
    int bad_field;
//CHECK:^^^^^^^^^ declaration not allowed in this context
//CHECK:^^^^^^^^^ non-static member variable declaration is not allowed inside extension.
}
