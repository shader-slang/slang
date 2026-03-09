// Cooperative matrices represent data distributed across a subgroup of
// threads. They cannot be packed into a per-thread AnyValue byte buffer
// because their storage model is fundamentally different from ordinary
// per-thread data.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "Impl:IFoo=0" -conformance "GoodImpl:IFoo=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "Impl:IFoo=0" -conformance "GoodImpl:IFoo=1"

using namespace linalg;

interface IFoo
{
    int get();
}

export struct Impl : IFoo
//CHECK:      ^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^ type 'Impl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator> m;
    int get() { return 1; }
}

export struct GoodImpl : IFoo
{
    float v;
    int get() { return int(v); }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = createDynamicObject<IFoo>(1, 0);
    outputBuffer[0] = f.get();
}
