// A default argument declared on an interface requirement is used when a call omits the
// argument, even if the satisfying method does not redeclare the default (issue #12640).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float func(float a = 1.0);
}

// Default only on the requirement: the call resolves to the requirement, whose default is used.
struct Foo : IFoo
{
    float func(float a) { return a; }
}

// Default also on the implementation: the call resolves to `Bar::func` (the concrete-preferred
// candidate), whose own default is used.
struct Bar : IFoo
{
    float func(float a = 2.0) { return a; }
}

interface IMixed
{
    float g(float a = 1.0, float b = 2.0);
}

// Impl redeclares only the second default. The call resolves to the requirement, so BOTH omitted
// arguments take the requirement's defaults (1.0, 2.0) — not the implementation's (_, 20.0).
struct Mixed : IMixed
{
    float g(float a, float b = 20.0) { return a - b; }
}

// Generic interface whose requirement default *expression* references the generic value
// parameter `K`. The implementation omits the default, so lowering must substitute `K` into the
// requirement's default through the `GenImpl<K>` specialization (the generic-argument
// substitution path that `_lowerSubstitutionEnv` handles).
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

// Interface subscript requirement with a defaulted index the implementation omits: exercises the
// subscript lowering branch, which resolves the requirement through the witness the same way.
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
    // K=5 substituted into the requirement's default expression `a = K`.
    // CHECK: 5.0
    outputBuffer[4] = gen.scaled();
    // CHECK: 3.0
    outputBuffer[5] = s[];
}
