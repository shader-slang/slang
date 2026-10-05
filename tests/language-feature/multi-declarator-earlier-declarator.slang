// A later declarator's initializer in a multi-declarator local declaration
// must see the earlier declarators of the same statement, including when the
// parser looks up a name to decide whether `<` starts generic arguments or a
// parenthesized name is a cast (gh-13428).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[1 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=buf
RWStructuredBuffer<int> buf;

typedef int T;
int g<let N : int>() { return N; }
static int j5 = 5;

// Groups at namespace and struct scope are parsed eagerly, before any semantic
// checking, and also see their earlier declarators.
namespace NS
{
static const int T = 4, m = (T) - 1;
static const int g = 1, n = g < 0;
}
struct Q
{
    static const int T = 4, m = (T) - 1;
    static const int g = 1, n = g < 0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int j = buf[0], k = j < 2;
    int k1 = buf[0], j1 = k1 < 2;
    int a = buf[0], j2 = buf[1], k2 = a + (j2 < 2);
    int j3 = buf[0], k3 = j3 < 2 ? 1 : 0;
    int j4 = buf[0], k4 = j4 < buf[1];
    int2 v = int2(buf[0], 1), w = int2(v.x < 2, 0);
    int c0 = 3, c1 = c0 < j, c2 = c1 < c0;
    const int d0 = buf[0], d1 = d0 < 2;

    // An earlier declarator that shadows a global type or generic must be
    // the one found by the later initializer.
    int T = buf[0] + 4, m = (T) - 1;
    int g = buf[0], n = g < 0;

    struct S { int x; } s0 = { buf[1] }, s1 = { s0.x < 6 };

    int e = 0;
    for (int i = buf[0], f = i < 2; i < 2; i++)
        e = f;

    // A use of a name before the statement that declares it in a group still
    // refers to the outer declaration.
    int y = j5;
    int j5 = 1, k5 = 2;

    buf[0] = k;
    buf[1] = j1;
    buf[2] = k2;
    buf[3] = k3;
    buf[4] = k4;
    buf[5] = w.x;
    buf[6] = c1;
    buf[7] = c2;
    buf[8] = d1;
    buf[9] = m;
    buf[10] = n;
    buf[11] = s1.x;
    buf[12] = e;
    buf[13] = y + j5 + k5;
    buf[14] = NS::m * 10 + NS::n;
    buf[15] = Q::m * 10 + Q::n;
}

// CHECK: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 4
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 8
// CHECK-NEXT: 30
// CHECK-NEXT: 30
