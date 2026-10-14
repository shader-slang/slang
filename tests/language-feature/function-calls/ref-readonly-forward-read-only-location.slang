// A memory location that the caller cannot write may still be passed to a `__ref_readonly`
// parameter: a `__ref_readonly` or `__constref` parameter, or a field, array element, or vector
// element of one.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

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
    return leaf(s.a) + leaf(s.b[1]) + leaf(s.v[2]) + leafS(s);
}

int forwardConstRef(__constref S s)
{
    return leaf(s.a) + leaf(s.b[1]) + leaf(s.v[2]) + leafS(s);
}

int forwardLegacy(const __ref S s)
{
    return leaf(s.a) + leaf(s.b[1]) + leaf(s.v[2]) + leafS(s);
}

int forwardScalar(__ref_readonly int x) { return leaf(x); }

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.a = 1;
    s.b[0] = 0;
    s.b[1] = 20;
    s.v = int4(0, 0, 300, 0);
    int x = 7;

    // CHECK: 1321
    outputBuffer[0] = forward(s);
    // CHECK: 1321
    outputBuffer[1] = forwardConstRef(s);
    // CHECK: 1321
    outputBuffer[2] = forwardLegacy(s);
    // CHECK: 7
    outputBuffer[3] = forwardScalar(x);
}
