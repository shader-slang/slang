//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry computeMain

// A declaration body inside a function body is parsed as at global scope, so a
// `<` in a member initializer must not check the incomplete enclosing type
// (gh-13430). A local `class` also reports parser-recovery errors, which this
// test does not check.

RWStructuredBuffer<int> buf;

struct S
{
    int x;
}

struct G
{
    static const int a = 1;
    static const int b = a<int>;
//CHECK:                 ^ expected a generic
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // `a<int>` gives the same diagnostic as in the global `G`.
    struct L
    {
        static const int a = 1;
        static const int b = a<int>;
//CHECK:                     ^ expected a generic
    };

    // Declarations that are not allowed here reach the normal diagnostic.
    extension S { static const int a = 1; static const bool b = a < 2; }
//CHECK:      ^ extension is not allowed here
    class C { static const int a = 1; static const bool b = a < 2; };
//CHECK:  ^ class is not allowed here

    buf[0] = 1;
}
