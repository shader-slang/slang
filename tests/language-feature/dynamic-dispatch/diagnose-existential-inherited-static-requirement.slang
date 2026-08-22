// Passing an interface `IV` as an explicit argument for a generic parameter constrained by a
// base interface `T : IBase` must be rejected before lowering: the argument denotes the
// existential `dyn IV`, which does not conform to `IBase`. Covers the generic form of the
// inherited-base static-requirement case in shader-slang/slang#12430 (the requirement is
// inherited from a base interface and returns the base's associated type).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain

interface IBase
{
    associatedtype Diff;
    static Diff makeZero();
}

interface IV : IBase
{
    float get();
}

struct V : IV
{
    typealias Diff = float;
    float value;
    float get() { return value; }
    static float makeZero() { return 0.0; }
}

__generic<T : IBase>
T.Diff getZero()
{
    return T.makeZero();
}

[numthreads(1, 1, 1)]
void computeMain(uint3 t : SV_DispatchThreadID)
{
    var z = getZero<IV>();
//CHECK:    ^^^^^^^ error E38029
//CHECK:    ^^^^^^^ type argument 'dyn IV' does not conform to the required interface 'IBase'
}
