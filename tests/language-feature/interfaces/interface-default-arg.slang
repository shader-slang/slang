// A call that omits an argument uses the default declared on the
// interface requirement, even when the implementation does not
// redeclare it (issue #12640).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float func(float a = 1.0);
}

// Default only on the requirement.
struct Foo : IFoo
{
    float func(float a) { return a; }
}

// Default also on the implementation, which overload resolution prefers.
struct Bar : IFoo
{
    float func(float a = 2.0) { return a; }
}

interface IMixed
{
    float g(float a = 1.0, float b = 2.0);
}

// Only the second default is redeclared; both omitted arguments still
// take the requirement's defaults.
struct Mixed : IMixed
{
    float g(float a, float b = 20.0) { return a - b; }
}

// The default keeps `K`'s substitution when the call resolves to an
// implementation that does not declare a default.
interface IGen<let K : int>
{
    float scaled(float a = K);
}

struct GenImpl<let K : int> : IGen<K>
{
    float scaled(float a) { return a; }
}

interface ISub
{
    __subscript(int i = 3) -> float { get; }
}

// Subscripts take the same path through a witness as method calls.
struct Sub : ISub
{
    __subscript(int i) -> float { get { return float(i); } }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f;
    Bar b;
    Mixed m;
    GenImpl<5> gen;
    Sub s;
    // CHECK: 1.0
    outputBuffer[0] = f.func();
    // CHECK: 2.0
    outputBuffer[1] = b.func();
    // CHECK: 5.0
    outputBuffer[2] = f.func(5.0);
    // CHECK: -1.0
    outputBuffer[3] = m.g();
    // CHECK: 5.0
    outputBuffer[4] = gen.scaled();
    // CHECK: 3.0
    outputBuffer[5] = s[];
}
