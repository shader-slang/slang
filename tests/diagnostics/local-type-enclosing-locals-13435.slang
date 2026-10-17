//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A function-local type cannot refer to the parameters or non-static local variables of the
// enclosing function. The type's members do not run in that function's frame, and capture
// belongs to lambdas, so lookup does not find those names from inside the type.

RWStructuredBuffer<int> buf;

[shader("compute")][numthreads(1, 1, 1)]
void main()
{
    int x = buf[0];
    struct L
    {
        int get() { return x + 1; }
//CHECK:                   ^ undefined identifier
//CHECK:                   ^ undefined identifier 'x'.
    };
    L l;
    buf[1] = l.get();
}

int staticConstInitializer()
{
    int x = buf[0];
    struct L { static const int N = x + 1; };
//CHECK:                            ^ undefined identifier
//CHECK:                            ^ undefined identifier 'x'.
    return L::N;
}

int fieldInitializer()
{
    int x = buf[0];
    struct L { int a = x; };
//CHECK:               ^ undefined identifier
//CHECK:               ^ undefined identifier 'x'.
    L l;
    return l.a;
}

int parameter(int p)
{
    struct L { int get() { return p + 1; } };
//CHECK:                          ^ undefined identifier
//CHECK:                          ^ undefined identifier 'p'.
    L l;
    return l.get();
}

int lambdaInMember()
{
    int x = buf[0];
    struct L { int get() { let f = () => x + 1; return f(); } };
//CHECK:                                 ^ undefined identifier
//CHECK:                                 ^ undefined identifier 'x'.
    L l;
    return l.get();
}

int constLocal()
{
    const int k = 3;
    struct L { int get() { return k; } };
//CHECK:                          ^ undefined identifier
//CHECK:                          ^ undefined identifier 'k'.
    L l;
    return l.get();
}

// The local `g` hides the global `g` from the local type, so the reference is an error rather
// than a silent reference to the global. C++ also rejects this; `::g` names the global.
static const int g = 7;
int shadowedGlobal()
{
    int g = buf[0];
    struct L { int get() { return g; } };
//CHECK:                          ^ undefined identifier
//CHECK:                          ^ undefined identifier 'g'.
    L l;
    return l.get();
}

int letLocal()
{
    let k = 4;
    struct L { int get() { return k; } };
//CHECK:                          ^ undefined identifier
//CHECK:                          ^ undefined identifier 'k'.
    L l;
    return l.get();
}

int genericLocalType(int p)
{
    struct L<T> { int get() { return p; } };
//CHECK:                             ^ undefined identifier
//CHECK:                             ^ undefined identifier 'p'.
    L<int> l;
    return l.get();
}

int lambdaParameter()
{
    let f = (int a) => { struct L { int get() { return a; } }; L l; return l.get(); };
//CHECK:                                               ^ undefined identifier
//CHECK:                                               ^ undefined identifier 'a'.
    return f(1);
}

// A function-local `cbuffer` is a local whose fields lookup finds through it. Nesting the local
// type in a lambda makes lookup reach the field through the `cbuffer`'s transparent local; a
// type written directly in the function takes a different lookup path.
int localCBufferFieldInLambda()
{
    cbuffer CB { int c; };
    let f = () => { struct L { int get() { return c; } }; L l; return l.get(); };
//CHECK:                                          ^ undefined identifier
//CHECK:                                          ^ undefined identifier 'c'.
    return f();
}

// A `const` local is still a non-static local, so a local type cannot use it in a `static const`
// initializer or a type either. `static const` is the spelling that stays visible.
int constLocalInStaticConstInitializer()
{
    const int N = 4;
    struct L { static const int M = N; };
//CHECK:                            ^ undefined identifier
//CHECK:                            ^ undefined identifier 'N'.
    return L::M;
}

int constLocalInType()
{
    const int N = 2;
    struct L { vector<int, N> v; };
//CHECK:                   ^ undefined identifier
//CHECK:                   ^ undefined identifier 'N'.
    L l;
    return l.v.x;
}

// `visitBlockStmt` checks local types before the rest of the block, so a local declared after
// the type also hides the global of the same name from it.
static const int h = 7;
int laterLocalHidesGlobal()
{
    struct L { int get() { return h; } };
//CHECK:                          ^ undefined identifier
//CHECK:                          ^ undefined identifier 'h'.
    L l;
    int h = l.get();
    return h;
}

// No "did you mean" note suggests a local that the local type cannot use, or a global that such
// a local hides.
static const int total = 1;
int noSuggestionOfShadowedGlobal()
{
    int total = buf[0];
    struct L { int get() { return totl; } };
//CHECK:                          ^^^^ undefined identifier
//CHECK:                          ^^^^ undefined identifier 'totl'.
    L l;
    return l.get() + total;
}

struct S
{
    int v;
    property int p
    {
        get { return v; }
        set { struct L { int get() { return newValue; } }; L l; v = l.get(); }
//CHECK:                                    ^^^^^^^^ undefined identifier
//CHECK:                                    ^^^^^^^^ undefined identifier 'newValue'.
    }
    __subscript(int i)->int
    {
        get { struct L { int get() { return i; } }; L l; return l.get(); }
//CHECK:                                    ^ undefined identifier
//CHECK:                                    ^ undefined identifier 'i'.
    }
};

// A global that no local hides is still suggested from inside a local type.
static const int counter = 3;
int suggestsUnshadowedGlobal()
{
    struct L { int get() { return countr; } };
//CHECK:                          ^^^^^^ undefined identifier
//CHECK:                          ^^^^^^ undefined identifier 'countr'.
//CHECK:                          ^^^^^^ did you mean 'counter'?
    L l;
    return l.get();
}
