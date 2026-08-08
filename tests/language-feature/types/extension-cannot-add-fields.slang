// extension-cannot-add-fields.slang

// Test that an extension cannot add data fields (non-static member variables) to a type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

struct Foo
{
    int x;
}

extension Foo
{
    int y;
//CHECK:^ declaration not allowed in this context
//CHECK:^ non-static member variable declaration is not allowed inside extension.
}
