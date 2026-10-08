//TEST:INTERPRET(filecheck=CHECK):

// A generic `where T == U` equality constraint must be usable in *both*
// directions -- a `T` where a `U` is expected and a `U` where a `T` is expected.
// This mirrors the interface-level `__constraint A == B` form
// (see tests/language-feature/interfaces/derived-interface-constraint-equal-assoc.slang);
// the two are different surface/discovery paths but must agree on behavior.

struct X
{
    int v;
}

U asU<T, U>(T t, U u) where T == U
{
    return t; // T used where U is expected
}

T asT<T, U>(T t, U u) where T == U
{
    return u; // U used where T is expected
}

void main()
{
    X a;
    a.v = 5;
    X b;
    b.v = 8;
    // CHECK: 5 8
    printf("%d %d\n", asU(a, b).v, asT(a, b).v);
}
