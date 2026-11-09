// Companion to gh-13260. When a static requirement's first parameter IS the conforming (`This`)
// type, a same-name instance method still satisfies it through the non-static-satisfies-static
// adaptation, and an explicit `override` of that requirement is chosen ahead of the interface
// default. Here `S.f()` (returns 2) must be the witness for `IFoo.f(This)`, not the default
// (returns 1). If the adaptation were wrongly declined for this shape, `override int f()` would be
// marked as overriding nothing (E30854) and would also lose to the default.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute

interface IFoo
{
    static int f(This value) { return 1; }
}

struct S : IFoo
{
    int x;
    override int f() { return 2; }
}

// `S : IFoo` is conformance-checked because `S` is declared; this call is what forces the chosen
// witness to actually be lowered and run, so the selected implementation is observable.
int callF<T : IFoo>(T v)
{
    return T.f(v);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

//SPV: OpEntryPoint
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.x = 5;
    outputBuffer[0] = callF<S>(s); // CHECK: 2
}
