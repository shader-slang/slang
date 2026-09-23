// non-exhaustive is required because each E31228 also emits a span row that no
// CHECK matches.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: constexpr on a function or other callable declaration is not a supported Slang feature and is ignored
//CHECK: constexpr on a function or other callable declaration is not a supported Slang feature and is ignored
//CHECK: constexpr on a function or other callable declaration is not a supported Slang feature and is ignored
//CHECK: constexpr on a function or other callable declaration is not a supported Slang feature and is ignored
//CHECK: constexpr on a function or other callable declaration is not a supported Slang feature and is ignored
//CHECK: constexpr on a function or other callable declaration is not a supported Slang feature and is ignored

struct S
{
    int v;

    constexpr __init(int x) { v = x; }

    constexpr int method() { return v; }

    constexpr __subscript(int i) -> int
    {
        constexpr get { return v; }
    }
}

constexpr S operator+(S a, S b) { return S(a.v + b.v); }

constexpr int freeFunc(int a) { return a; }

[numthreads(1, 1, 1)]
void computeMain()
{
    S s = S(1) + S(2);
    int x = s.method() + s[0] + freeFunc(3);
}
