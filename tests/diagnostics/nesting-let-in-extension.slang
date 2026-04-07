//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// `let` (non-static) in an extension is also an instance variable,
// and should be rejected.

struct T { int x; }

extension T
{
    let bad_let : int = 0;
//CHECK:^^^^^^^ declaration not allowed in this context
//CHECK:^^^^^^^ non-static member variable declaration is not allowed inside extension.
}
