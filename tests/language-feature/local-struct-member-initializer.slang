// The member initializers of a type declared inside a function body are parsed
// the same way as those of a global type: a name used in `a < 2` or `(a)` must
// not make the parser check the type while its body is still being parsed
// (gh-13430).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

#pragma warning(disable:30816)

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=buf
RWStructuredBuffer<int> buf;

int f<let N : int>() { return N; }

struct B0
{
    int z = 3;
}

// Global controls with the same members as the local types below.
struct G
{
    int a = 1;
    int b = a < 2;
    int c = (a) + 2;
    int d = f<2>();
}

struct GG<T>
{
    int a = 1;
    int b = a < 2;
}

struct GS
{
    static const int a = 1;
    static const int b = GS::a < 2;
}

struct GB : B0
{
    int a = 1;
    int b = a < 2;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    struct L
    {
        int a = 1;
        int b = a < 2;
        int c = (a) + 2;
        int d = f<2>();
    }

    struct LG<T>
    {
        int a = 1;
        int b = a < 2;
    }

    struct LS
    {
        static const int a = 1;
        static const int b = LS::a < 2;
    }

    struct LB : B0
    {
        int a = 1;
        int b = a < 2;
    }

    G g;
    L l;
    GG<int> gg;
    LG<int> lg;
    GB gb;
    LB lb;

    buf[0] = l.b;
    buf[1] = g.b;
    buf[2] = l.c;
    buf[3] = g.c;
    buf[4] = l.d;
    buf[5] = g.d;
    buf[6] = lg.b;
    buf[7] = gg.b;
    buf[8] = LS::b;
    buf[9] = GS::b;
    buf[10] = lb.b + lb.z;
    buf[11] = gb.b + gb.z;
}

// CHECK: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 3
// CHECK-NEXT: 3
// CHECK-NEXT: 2
// CHECK-NEXT: 2
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 4
// CHECK-NEXT: 4
