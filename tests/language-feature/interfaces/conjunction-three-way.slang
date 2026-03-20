// conjunction-three-way.slang

// Test that three or more interfaces can be combined with `&`
// and the conjunction is correctly flattened.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IAlpha
{
    int getAlpha();
}

interface IBeta
{
    int getBeta();
}

interface IGamma
{
    int getGamma();
}

// Three-way conjunction in a generic constraint.
int sumAll<T : IAlpha & IBeta & IGamma>(T val)
{
    return val.getAlpha() + val.getBeta() + val.getGamma();
}

// Two-way subset conjunctions — should also work when the type satisfies all three.
int sumAB<T : IAlpha & IBeta>(T val)
{
    return val.getAlpha() + val.getBeta();
}

int sumBG<T : IBeta & IGamma>(T val)
{
    return val.getBeta() + val.getGamma();
}

int sumAG<T : IAlpha & IGamma>(T val)
{
    return val.getAlpha() + val.getGamma();
}

// Three-way conjunction in an inheritance clause.
struct Triple : IAlpha & IBeta & IGamma
{
    int a;
    int b;
    int c;

    int getAlpha() { return a; }
    int getBeta()  { return b; }
    int getGamma() { return c; }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// CHECK: 6
// CHECK: 3
// CHECK: 5
// CHECK: 4
// CHECK: 9
// CHECK: 5
// CHECK: 7
// CHECK: 6
// CHECK: 12
// CHECK: 7
// CHECK: 9
// CHECK: 8
// CHECK: 15
// CHECK: 9
// CHECK: 11
// CHECK: 10

[numthreads(4, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    Triple t;
    t.a = tid.x + 1;
    t.b = tid.x + 2;
    t.c = tid.x + 3;
    int base = tid.x * 4;
    outputBuffer[base + 0] = sumAll(t);  // a+b+c
    outputBuffer[base + 1] = sumAB(t);   // a+b
    outputBuffer[base + 2] = sumBG(t);   // b+c
    outputBuffer[base + 3] = sumAG(t);   // a+c
}
