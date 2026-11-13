// A `groupshared` parameter of an interface method called through dynamic dispatch. The parameter's
// IR type is `RateQualified(GroupShared, RefParam(T))`, and type-flow specialization compares the
// parameter's direction in the requirement's function type with the direction derived from each
// implementation's parameter, which carries no rate. Comparing them without looking through the
// rate reported `In` on one side and `Ref` on the other (an internal compiler error). The
// dispatched callees must write the group-shared array itself, with no per-invocation copy
// (shader-slang/slang#10641).

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "Add:IFill=0" -conformance "Mul:IFill=1"
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry computeMain -conformance "Add:IFill=0" -conformance "Mul:IFill=1"
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain -conformance "Add:IFill=0" -conformance "Mul:IFill=1"
//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain -conformance "Add:IFill=0" -conformance "Mul:IFill=1"

static const uint N = 8;
groupshared uint groupSharedData[N];
RWStructuredBuffer<uint> outputBuffer;

interface IFill
{
    void fill(groupshared uint s[N], uint i);
}

struct Add : IFill
{
    uint k;
    void fill(groupshared uint s[N], uint i) { s[i] = i + k; }
}

struct Mul : IFill
{
    uint k;
    void fill(groupshared uint s[N], uint i) { s[i] = i * k; }
}

[numthreads(N, 1, 1)]
void computeMain(uint i : SV_GroupIndex, uniform IFill filler)
{
    filler.fill(groupSharedData, i);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[i] = groupSharedData[(i + 1) % N];
}

// The dispatch function's cases may be emitted in either order.
// HLSL: groupshared uint [[SHARED:[A-Za-z0-9_]+]][
// HLSL-NOT: uint {{[A-Za-z0-9_]+}}[int(8)] = [[SHARED]]
// HLSL-DAG: [[SHARED]][{{.*}}] = {{.*}} +
// HLSL-DAG: [[SHARED]][{{.*}}] = {{.*}} *
// HLSL-NOT: uint {{[A-Za-z0-9_]+}}[int(8)] = [[SHARED]]
// SPIRV: OpEntryPoint
// GLSL: void main()
// MTL: void computeMain
