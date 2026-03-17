// Explicit [anyValueSize] with two scenarios:
// 1. Exact fit: [anyValueSize(8)] where the largest impl is exactly 8 bytes.
// 2. Oversized: [anyValueSize(32)] where the largest impl is only 8 bytes.
// Both should dispatch correctly without errors.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[anyValueSize(8)]
interface IExact
{
    int getValue();
}

struct ExactSmall : IExact
{
    int v;
    int getValue() { return v; }
}

struct ExactFull : IExact
{
    int a;
    int b;
    int getValue() { return a + b; }
}

[anyValueSize(32)]
interface IOversized
{
    int getValue();
}

struct OversizedSmall : IOversized
{
    int v;
    int getValue() { return v; }
}

struct OversizedFull : IOversized
{
    int a;
    int b;
    int getValue() { return a + b; }
}

IExact makeExact(int id)
{
    if (id == 0)
    {
        ExactSmall s;
        s.v = 10;
        return s;
    }
    else
    {
        ExactFull f;
        f.a = 20;
        f.b = 30;
        return f;
    }
}

IOversized makeOversized(int id)
{
    if (id == 0)
    {
        OversizedSmall s;
        s.v = 100;
        return s;
    }
    else
    {
        OversizedFull f;
        f.a = 200;
        f.b = 300;
        return f;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Exact-fit interface: small impl returns 10
    var a = makeExact(0);
    outputBuffer[0] = a.getValue();
    // CHECK: 10

    // Exact-fit interface: full impl returns 20 + 30 = 50
    var b = makeExact(1);
    outputBuffer[1] = b.getValue();
    // CHECK: 50

    // Oversized interface: small impl returns 100
    var c = makeOversized(0);
    outputBuffer[2] = c.getValue();
    // CHECK: 100

    // Oversized interface: full impl returns 200 + 300 = 500
    var d = makeOversized(1);
    outputBuffer[3] = d.getValue();
    // CHECK: 500
}
