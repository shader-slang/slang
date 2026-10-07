//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target cuda -entry computeMain -stage compute

// Regression guard for https://github.com/shader-slang/slang/issues/10641.
//
// An `out`/`inout` parameter that is not `groupshared` has copy-in/copy-out semantics, which an
// implementation may realize by reference only by assuming the argument is not aliased. Passing
// thread-group-shared storage, which every invocation in the group can access, is warning 30709.
// A direct argument of a call that is always inlined crosses no call boundary, which keeps
// `g[i] += 1` and `g[i]++` silent; an implicitly converted argument still goes through a temporary.
//
// The `//diag:` annotations are caret-less substring matches, and the test is exhaustive, so the
// silent calls in `computeMain` must produce no diagnostic at all.

struct Counter
{
    uint n;

    [mutating]
    void bump()
    {
        n++;
    }
}

groupshared int g_a[10];
groupshared uint g_u[10];
groupshared Counter g_c;

void myFunc(inout int a[10])
{
    a[0] = 1;
}

void setOne(inout int v)
{
    v = 1;
}

void writeOut(out int v)
{
    v = 2;
}

void setUint(inout uint v)
{
    v = 3;
}

[ForceInline]
void inlinedSet(inout uint v)
{
    v = 4;
}

void readIn(int v) {}

void share(groupshared int v)
{
    v = 5;
}

[numthreads(10, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    // --- Warned ---

    // The whole group-shared array, an element of it, and an `out` parameter.
    myFunc(g_a);
    setOne(g_a[i]);
    writeOut(g_a[i]);

    // A field of a group-shared struct.
    setUint(g_c.n);

    // An implicitly converted element, to an ordinary and to an inlined callee.
    setUint(g_a[i]);
    inlinedSet(g_a[i]);

    // The implicit `this` of a `[mutating]` method.
    g_c.bump();

    // --- Silent ---

    g_a[i] += 1;
    g_a[i]++;
    inlinedSet(g_u[i]);
    readIn(g_a[i]);
    share(g_a[i]);
    int local = 0;
    setOne(local);
}

//diag: but 'a' is an 'out' or 'inout' parameter
//diag: but 'v' is an 'out' or 'inout' parameter
//diag: but 'v' is an 'out' or 'inout' parameter
//diag: but 'v' is an 'out' or 'inout' parameter
//diag: but 'v' is an 'out' or 'inout' parameter
//diag: but 'v' is an 'out' or 'inout' parameter
//diag: but 'this' is an 'out' or 'inout' parameter
