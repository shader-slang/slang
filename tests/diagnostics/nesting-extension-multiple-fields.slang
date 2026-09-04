//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Multiple non-static fields in an extension should each get their own diagnostic.

struct Target { int original; }

extension Target
{
    int field1;
//CHECK:^^^^^^ error E31400
//CHECK:^^^^^^ non-static member variable declaration is not allowed inside extension.

    float field2;
//CHECK:  ^^^^^^ error E31400
//CHECK:  ^^^^^^ non-static member variable declaration is not allowed inside extension.

    bool field3;
//CHECK: ^^^^^^ error E31400
//CHECK: ^^^^^^ non-static member variable declaration is not allowed inside extension.

    // But static fields are fine:
    static int ok_static;
}
