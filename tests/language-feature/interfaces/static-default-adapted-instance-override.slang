// Companion to gh-13260: the fix that prefers an inherited default over the
// non-static-satisfies-static adaptation must still let a genuine receiver-style adaptation win.
// Here the static requirement's first parameter IS the conforming (`This`) type, so an explicit
// instance `override` of it binds through the adaptation and takes precedence over the default.
// A regression would surface as E30854 "'f' marked as 'override' is not overriding any base
// declarations" when S's conformance is checked.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

interface IFoo
{
    static int f(This value) { return 1; }
}

struct S : IFoo
{
    int x;
    override int f() { return 2; }
}

// Calling the static `f(This)` requirement through a generic uses S's witness for it, forcing the
// conformance check (and the `override` marking) that the guard must not break.
int callF<T : IFoo>(T v)
{
    return T.f(v);
}

RWStructuredBuffer<int> outputBuffer;

//CHECK: OpEntryPoint
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.x = 5;
    outputBuffer[0] = callF<S>(s);
}
