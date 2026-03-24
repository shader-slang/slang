// Stress test for switch-based dynamic dispatch with 8 conforming types.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

interface ICalc
{
    int calc(int x);
}

struct Impl0 : ICalc { int calc(int x) { return x; } }
struct Impl1 : ICalc { int calc(int x) { return x + 1; } }
struct Impl2 : ICalc { int calc(int x) { return x + 2; } }
struct Impl3 : ICalc { int calc(int x) { return x + 3; } }
struct Impl4 : ICalc { int calc(int x) { return x + 4; } }
struct Impl5 : ICalc { int calc(int x) { return x + 5; } }
struct Impl6 : ICalc { int calc(int x) { return x + 6; } }
struct Impl7 : ICalc { int calc(int x) { return x + 7; } }

ICalc makeCalc(int id)
{
    switch (id)
    {
    case 0: return Impl0();
    case 1: return Impl1();
    case 2: return Impl2();
    case 3: return Impl3();
    case 4: return Impl4();
    case 5: return Impl5();
    case 6: return Impl6();
    default: return Impl7();
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    for (int i = 0; i < 8; i++)
    {
        ICalc obj = makeCalc(i);
        outputBuffer[i] = obj.calc(10);
    }
    // CHECK: 10
    // CHECK: 11
    // CHECK: 12
    // CHECK: 13
    // CHECK: 14
    // CHECK: 15
    // CHECK: 16
    // CHECK: 17
}
