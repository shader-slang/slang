// A type parameter with an *optional* interface constraint (`where optional T : IFoo`) does not
// require conformance, so an explicit interface argument to it is NOT treated as the existential
// `dyn IFoo`. It is still rejected — an interface is not statically resolvable as a generic
// argument — but via the pre-existing existential-specialization diagnostic (E33180), not the
// conformance diagnostic used for required constraints. This guards the optional-constraint
// carve-out in shader-slang/slang#12430.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain

interface IFoo { void thing(); }
struct A : IFoo { void thing() {} }

int useIt<T>(T t) where optional T : IFoo { return 0; }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    A a;
    outputBuffer[0] = useIt<IFoo>(a);
//CHECK:                         ^ error E33180
//CHECK:                         ^ specializing 'useIt' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time.
}
