//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Static variables are allowed in struct, interface, extension, enum, and module.
// None of these should produce nesting-validation diagnostics.

static int moduleStatic;

struct S
{
    static int sStructVar;
}

interface IFoo
{
    static const int sIfaceVar = 42;
}

struct Base { int x; }
extension Base
{
    static int sExtVar;
}

enum E
{
    A,
    static int sEnumVar;
}
