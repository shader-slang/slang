//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for https://github.com/shader-slang/slang/issues/13426
//
// A `<` after a value that does not check to a DeclRefExpr (a swizzle, a tuple
// element, a member reached through a temporary) must parse as less-than, even when
// a later `>` is followed by `(`. The file also guards two details of the fix:
// - rows 8-9 (`p->y`, `p->x`): the checked base is reused, so the `DerefMemberExpr`
//   is not checked a second time (which reported a false "cannot dereference");
// - row 18 (`h.o.get<4>()`): a base of unknown kind is not reused, because generic
//   application rejects the `LetExpr` it checks to.
//
// Inputs: uv = (0.25, 0.75), x = 0.75, a = 0.5 lies between uv.x and uv.y, and b = 0.75
// equals uv.y and x, so a strict comparison of those against b never holds.

//TEST_INPUT:ubuffer(data=[0.25 0.75 0.5 0.75], stride=4):name=inputs
RWStructuredBuffer<float> inputs;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct S
{
    float2 v;
}

S getS()
{
    S s;
    s.v = float2(inputs[0], inputs[1]);
    return s;
}

interface IVal
{
    property float val { get; }
    int get<let N : int>();
}

struct V : IVal
{
    float x;
    property float val { get { return x; } }
    int get<let N : int>() { return N; }
}

IVal makeVal()
{
    V v;
    v.x = inputs[1];
    return v;
}

struct H
{
    static int sget<T>() { return 10; }
    V f;
    IVal o;
}

int genericSwizzle<T : __BuiltinFloatingPointType>(vector<T, 2> v, T a)
{
    return (v.y < a || v.y > (a)) ? 1 : 0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float2 uv = float2(inputs[0], inputs[1]);
    float a = inputs[2];
    float b = inputs[3];
    float x = inputs[1];
    float3x3 m = float3x3(inputs[1], 0, 0, 0, 1, 0, 0, 0, 1);
    Tuple<float, float> t = makeTuple(inputs[0], inputs[1]);
    S s;
    s.v = uv;
    float2* p = &uv;

    // CHECK: 1
    outputBuffer[0] = (uv.y < a || uv.y > (a)) ? 1 : 0;
    // CHECK-NEXT: 0
    outputBuffer[1] = (uv.y < b || uv.y > (b)) ? 1 : 0;
    // CHECK-NEXT: 0
    outputBuffer[2] = (uv.y < a && uv.y > (a)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[3] = (x.x < a || x.x > (a)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[4] = (m._m00 < a || m._m00 > (a)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[5] = (t._0 < a || t._1 > (a)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[6] = (s.v.x < a || s.v.y > (a)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[7] = (getS().v.y < a || getS().v.y > (a)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[8] = (p->y < a || p->y > (a)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[9] = (p->x < a) ? 1 : 0;

    // CHECK-NEXT: 1
    outputBuffer[10] = (uv.x < a && uv.y > (a)) ? 1 : 0;
    // CHECK-NEXT: 0
    outputBuffer[11] = (x.x < b || x.x > (b)) ? 1 : 0;
    // CHECK-NEXT: 0
    outputBuffer[12] = (m._m00 < b || m._m00 > (b)) ? 1 : 0;
    // CHECK-NEXT: 0
    outputBuffer[13] = (t._1 < b || t._1 > (b)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[14] = (makeVal().val < a || makeVal().val > (a)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[15] = genericSwizzle<float>(uv, a);
    let f = () => (uv.y < a || uv.y > (a)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[16] = f();

    // Genuine generic members must still parse as generic applications.
    H h;
    h.o = makeVal();
    // CHECK-NEXT: 13
    outputBuffer[17] = h.f.get<3>() + H::sget<int>();
    // CHECK-NEXT: 4
    outputBuffer[18] = h.o.get<4>();
}
