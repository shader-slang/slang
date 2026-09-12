// Passing an interface `IV` as an explicit argument for a generic parameter constrained
// `T : IV` must be rejected: the argument denotes the existential `dyn IV`, which does not
// conform to `IV`. See shader-slang/slang#12430.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain

interface IV
{
    associatedtype Assoc;
    static Assoc makeIt();
}

struct V : IV
{
    typealias Assoc = float;
    static float makeIt() { return 1.0; }
}

__generic<T : IV>
T.Assoc callStatic()
{
    return T.makeIt();
}

[numthreads(1, 1, 1)]
void computeMain(uint3 t : SV_DispatchThreadID)
{
    var r = callStatic<IV>();
//CHECK:    ^^^^^^^^^^ error E38029
//CHECK:    ^^^^^^^^^^ type argument 'dyn IV' does not conform to the required interface 'IV'
}
