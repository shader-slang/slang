// The member initializers of a type declared inside a function body are parsed
// the same way as those of a global type: a name used in `a < 2` or `(a)` must
// not make the parser check the type while its body is still being parsed
// (gh-13430). Each local type `L*` has a global twin `G*` that must give the
// same result. The rows use `static const` members because an instance member
// whose default reads another member crashes for a global struct too, which is
// a separate bug; the issue's instance-member spellings are only compiled.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// `LB` and `GB` inherit from a struct, which warns.
#pragma warning(disable:30816)

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=buf
RWStructuredBuffer<int> buf;

int f<let N : int>() { return N; }

struct B0
{
    int z;
};

struct G
{
    static const int a = 1;
    static const bool b = a < 2;
    static const int c = (a) + 2;
};

struct GG<T>
{
    static const int a = 1;
    static const bool b = a < 2;
};

struct GS
{
    static const int a = 1;
    static const bool b = GS::a < 2;
};

struct GB : B0
{
    static const int a = 1;
    static const bool b = a < 2;
};

struct GF
{
    int d = f<2>();
    int e = f<2>() < 3;
};

struct GC
{
    static const int a = 1, b = a < 2;
};

[numthreads(1, 1, 1)]
void computeMain()
{
    // The issue's own spellings only need to compile.
    struct I0 { int a = 1; int b = a < 2; };
    struct I1 { int a = 1; int b = (a); };
    struct I2 { int a = 1, b = a < 2; };
    cbuffer CB { int a = 1; int b = a < 2; }

    // Generic disambiguation and cast detection must not check the incomplete type.
    struct L
    {
        static const int a = 1;
        static const bool b = a < 2;
        static const int c = (a) + 2;
    };

    // The `GenericDecl` that wraps a generic local type is incomplete too.
    struct LG<T>
    {
        static const int a = 1;
        static const bool b = a < 2;
    };

    // Qualified lookup must not check the incomplete local type.
    struct LS
    {
        static const int a = 1;
        static const bool b = LS::a < 2;
    };

    // Inheritance must not force early checking of the local type.
    struct LB : B0
    {
        static const int a = 1;
        static const bool b = a < 2;
    };

    // The heuristic must still recognize generic calls inside a local type.
    struct LF
    {
        int d = f<2>();
        int e = f<2>() < 3;
    };

    // Parsing the second initializer must not check the still-incomplete type.
    struct LC
    {
        static const int a = 1, b = a < 2;
    };

    GF gf = {};
    LF lf = {};

    buf[0] = int(L::b);
    buf[1] = int(G::b);
    buf[2] = L::c;
    buf[3] = G::c;
    buf[4] = int(LG<int>::b);
    buf[5] = int(GG<int>::b);
    buf[6] = int(LS::b);
    buf[7] = int(GS::b);
    buf[8] = int(LB::b);
    buf[9] = int(GB::b);
    buf[10] = lf.d;
    buf[11] = gf.d;
    buf[12] = lf.e;
    buf[13] = gf.e;
    buf[14] = LC::b;
    buf[15] = GC::b;
}

// CHECK: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 3
// CHECK-NEXT: 3
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 2
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
