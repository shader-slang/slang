//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Constructors are allowed in struct, extension, and enum.

struct S
{
    int x;
    __init(int v) { x = v; }
}

extension S
{
    __init() { x = 0; }
}
