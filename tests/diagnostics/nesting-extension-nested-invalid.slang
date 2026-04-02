//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Extension inside a struct is not allowed (caught by parser).
// Extension inside another extension is also not allowed.

struct S1 { int x; }
struct S2 { int y; }

struct Container
{
    extension S1 {}
//CHECK:      ^^ declaration not allowed here
//CHECK:      ^^ extension is not allowed here.
}

extension S1
{
    extension S2 {}
//CHECK:      ^^ declaration not allowed here
//CHECK:      ^^ extension is not allowed here.
}
