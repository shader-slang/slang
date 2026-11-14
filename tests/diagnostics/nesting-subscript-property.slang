//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Subscript and property containers only allow accessors (and empty decls).

struct S
{
    // Valid: accessors inside subscript
    __subscript(int i) -> int
    {
        get { return 0; }
        set {}
    }

    // Valid: accessors inside property
    property int p
    {
        get { return 0; }
        set(int v) {}
    }
}
