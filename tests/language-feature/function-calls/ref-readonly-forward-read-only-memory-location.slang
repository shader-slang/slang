// A memory location that the caller cannot write may still be passed to a `__ref_readonly`
// parameter: a `__ref_readonly` parameter, or a field, array element, vector element, matrix
// element, tuple element, or swizzle of one. A `__constref` parameter is a borrow rather than a memory location, but
// the body may write to it like any `in` parameter, so its fields are passed as l-values.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -experimental-feature

struct S
{
    int a;
    int b[2];
    int4 v;
}

int leaf(__ref_readonly int x) { return x; }
int leafS(__ref_readonly S s) { return s.a * 1000; }

int forward(__ref_readonly S s)
{
    return leaf(s.a) + leaf(s.b[1]) + leaf(s.v[2]) + leaf(s.v.y) + leafS(s);
}

int forwardConstRef(__constref S s)
{
    return leaf(s.a) + leaf(s.b[1]) + leaf(s.v[2]) + leaf(s.v.y) + leafS(s);
}

int forwardLegacy(const __ref S s)
{
    return leaf(s.a) + leaf(s.b[1]) + leaf(s.v[2]) + leaf(s.v.y) + leafS(s);
}

int forwardScalar(__ref_readonly int x) { return leaf(x); }

int forwardMatrixElement(__ref_readonly int4x4 m) { return leaf(m._m12); }
int forwardTupleElement(__ref_readonly Tuple<int, int> t) { return leaf(t._1); }

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.a = 1;
    s.b[0] = 0;
    s.b[1] = 20;
    s.v = int4(0, 4000, 300, 0);
    int x = 7;

    // CHECK: 5321
    outputBuffer[0] = forward(s);
    // CHECK: 5321
    outputBuffer[1] = forwardConstRef(s);
    // CHECK: 5321
    outputBuffer[2] = forwardLegacy(s);
    // CHECK: 7
    outputBuffer[3] = forwardScalar(x);

    int4x4 m = int4x4(0);
    m[1][2] = 12;
    Tuple<int, int> t = makeTuple(1, 9);
    // CHECK: 12
    outputBuffer[4] = forwardMatrixElement(m);
    // CHECK: 9
    outputBuffer[5] = forwardTupleElement(t);
}
