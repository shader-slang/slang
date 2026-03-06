// Cooperative matrices represent data distributed across a subgroup of
// threads. They cannot be packed into a per-thread AnyValue byte buffer
// because their storage model is fundamentally different from ordinary
// per-thread data.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

using namespace linalg;

interface IFoo
{
    int get();
}

// CHECK: type 'Impl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct Impl : IFoo
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
    IFoo f = createDynamicObject<IFoo>(0, 0);
    outputBuffer[0] = f.get();
}
