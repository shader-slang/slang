//DIAGNOSTIC_TEST:SIMPLE(diag=WARNING):-target hlsl -entry computeMain -conformance A:IOuter=0 -conformance B:IOuter=1
//DIAGNOSTIC_TEST:SIMPLE(diag=ERROR):-target hlsl -entry computeMain -conformance A:IOuter=0 -conformance B:IOuter=1 -warnings-as-errors 55219

interface IValue { uint get(); }
struct X : IValue { uint get() { return 2; } }
struct Y : IValue { uint get() { return 5; } }
interface IOuter { associatedtype Inner : IValue; Inner make(); }
struct A : IOuter { typealias Inner = X; X make() { return X(); } }
struct B : IOuter { typealias Inner = Y; Y make() { return Y(); } }
// The second witness constrains T.Inner, not T. Both constraints are valid,
// but choosing a concrete T alone does not establish this witness relationship.
[__specializePerConformance]
uint helper<T : IOuter>(T value) where T.Inner : IValue { return value.make().get(); }
RWStructuredBuffer<uint> output;
[shader("compute")][numthreads(1,1,1)]
void computeMain(uint3 tid:SV_DispatchThreadID) { output[tid.x] = helper(createDynamicObject<IOuter>(tid.x, 0u)); }
//WARNING: warning E55219
//WARNING: witness constraints do not all belong to the dynamic type parameter
//ERROR: error E55219
//ERROR: witness constraints do not all belong to the dynamic type parameter
