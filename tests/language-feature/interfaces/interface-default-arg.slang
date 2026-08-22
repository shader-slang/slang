// A default argument declared on an interface requirement is used when a call omits the
// argument, even if the satisfying method does not redeclare the default (issue #12640).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float func(float a = 1.0);
}

struct Foo : IFoo
{
    float func(float a) { return a; }
}

struct Bar : IFoo
{
    float func(float a = 2.0) { return a; }
}

interface IMixed
{
    float g(float a = 1.0, float b = 2.0);
}

// Impl redeclares only the second default. Since the call resolves against the requirement,
// both omitted arguments must take the requirement's defaults (1.0, 2.0), not (1.0, 20.0).
struct Mixed : IMixed
{
    float g(float a, float b = 20.0) { return a - b; }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f;
    Bar b;
    Mixed m;
    // CHECK: 1.0
    outputBuffer[0] = f.func();
    // CHECK: 2.0
    outputBuffer[1] = b.func();
    // CHECK: 5.0
    outputBuffer[2] = f.func(5.0);
    // CHECK: -1.0
    outputBuffer[3] = m.g();
}
