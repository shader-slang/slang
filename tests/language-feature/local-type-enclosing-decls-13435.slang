//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// A function-local type can still use the enclosing function's local types, its own name,
// the function's generic parameters, `static` locals, and global declarations. Only parameters
// and non-static local variables are inaccessible (see
// tests/diagnostics/local-type-enclosing-locals-13435.slang).

void f()
{
    struct A { static const int K = 3; };
    struct B
    {
        A a;                                  // an earlier local type
        static const int N = A::K + 1;
        static B make() { B b; return b; }    // the type's own name
    };
}

int localTypes()
{
    struct A { static const int K = 3; };
    struct B
    {
        A a;
        static const int N = A::K + 1;
        static B make() { B b; b.v = N; return b; }
        int v;
    };
    return B.make().v;
}

int staticConstLocal()
{
    static const int S = 5;
    struct L { int get() { return S + 1; } };
    L l;
    return l.get();
}

int staticLocal()
{
    static int counter = 9;
    struct L { int get() { return counter; } };
    L l;
    return l.get();
}

int genericValueParam<let N : int>()
{
    struct L { int get() { return N * 2; } };
    L l;
    return l.get();
}

int genericTypeParam<T : IInteger>(T t)
{
    struct L { T v; int get() { return v.toInt() + 1; } };
    L l;
    l.v = t;
    return l.get();
}

static const int globalConst = 20;
int globalFunc() { return 1; }

int unshadowedGlobal(int p)
{
    struct L { int get() { return globalConst + globalFunc(); } };
    L l;
    return l.get() + p;
}

// A local `x` hides the global `x` from the local type, and `::x` names the global explicitly.
static int x = 7;
int qualifiedGlobal()
{
    int x = 100;
    struct L { int get() { return ::x; } };
    L l;
    return l.get() + x;
}

// A lambda inside a member of a local type captures the member's own parameter, local and `this`.
int lambdaInLocalTypeMember(int p)
{
    struct L
    {
        int m;
        int get(int q)
        {
            int r = q + 1;
            let f = (int a) => a + q + r + m;
            return f(2);
        }
    };
    L l;
    l.m = 1;
    return l.get(p);
}

// A lambda written directly in the function still captures its locals and parameters.
int lambdaCapture(int p)
{
    int x = p + 1;
    let g = () => x + p;
    return g();
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    f();
    outputBuffer[0] = localTypes();
    outputBuffer[1] = staticConstLocal();
    outputBuffer[2] = staticLocal();
    outputBuffer[3] = genericValueParam<4>();
    outputBuffer[4] = genericTypeParam(int(10));
    outputBuffer[5] = lambdaCapture(2);
    outputBuffer[6] = unshadowedGlobal(0);
    outputBuffer[7] = qualifiedGlobal();
    outputBuffer[8] = lambdaInLocalTypeMember(3);
    // CHECK: 4
    // CHECK-NEXT: 6
    // CHECK-NEXT: 9
    // CHECK-NEXT: 8
    // CHECK-NEXT: 11
    // CHECK-NEXT: 5
    // CHECK-NEXT: 21
    // CHECK-NEXT: 107
    // CHECK-NEXT: 10
}
