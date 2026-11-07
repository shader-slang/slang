//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target cuda -entry computeMain -stage compute

// An `out` or `inout` parameter needs exclusive access to its argument during the call. Passing
// thread-group-shared storage, which every invocation in the group can access, is warning 30709,
// because the compiler cannot tell whether another invocation accesses that storage during the call
// (shader-slang/slang#10641). A direct argument of a mutating operator is read, modified and
// written in place as the user expects, which keeps `g[i] += 1` and `g[i]++` silent. A call by
// name warns even when it is always inlined or is a built-in. The test is exhaustive, so the
// silent calls must produce no diagnostic.

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

    // A direct argument of an inlined callee, and the `out` argument of a built-in.
    inlinedSet(g_u[i]);
    InterlockedAdd(g_u[0], 1u, g_u[1]);

    // The implicit `this` of a `[mutating]` method.
    g_c.bump();

    // --- Silent ---

    g_a[i] += 1;
    g_a[i]++;
    readIn(g_a[i]);
    share(g_a[i]);
    int local = 0;
    setOne(local);
}

//diag: 'myFunc' needs exclusive access to parameter 'a', but 'g_a' is thread-group-shared memory that other invocations could access
//diag: 'setOne' needs exclusive access to parameter 'v', but 'g_a[i]' is thread-group-shared memory that other invocations could access
//diag: 'writeOut' needs exclusive access to parameter 'v', but 'g_a[i]' is thread-group-shared memory that other invocations could access
//diag: 'setUint' needs exclusive access to parameter 'v', but 'g_c.n' is thread-group-shared memory that other invocations could access
//diag: 'setUint' needs exclusive access to parameter 'v', but 'g_a[i]' is thread-group-shared memory that other invocations could access
//diag: 'inlinedSet' needs exclusive access to parameter 'v', but 'g_a[i]' is thread-group-shared memory that other invocations could access
//diag: 'inlinedSet' needs exclusive access to parameter 'v', but 'g_u[i]' is thread-group-shared memory that other invocations could access
//diag: 'InterlockedAdd' needs exclusive access to parameter 'original_value', but 'g_u[1]' is thread-group-shared memory that other invocations could access
//diag: 'bump' needs exclusive access to parameter 'this', but 'g_c' is thread-group-shared memory that other invocations could access
//diag: consider copying value through a local variable to make the access explicit
//diag: consider copying value through a local variable to make the access explicit
//diag: consider copying value through a local variable to make the access explicit
//diag: consider copying value through a local variable to make the access explicit
//diag: consider copying value through a local variable to make the access explicit
//diag: consider copying value through a local variable to make the access explicit
//diag: consider copying value through a local variable to make the access explicit
//diag: consider copying value through a local variable to make the access explicit
//diag: consider copying value through a local variable to make the access explicit
//diag: 'inout' requires exclusive access
//diag: 'inout' requires exclusive access
//diag: 'inout' requires exclusive access
//diag: 'inout' requires exclusive access
//diag: 'inout' requires exclusive access
//diag: 'inout' requires exclusive access
//diag: 'out' requires exclusive access
//diag: 'out' requires exclusive access
//diag: '[mutating]' requires exclusive access to 'this'
