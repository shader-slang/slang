//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -conformance A:IValue=3 -conformance B:IValue=7
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -conformance A:IValue=3 -conformance B:IValue=7 -warnings-as-errors 55219

interface IValue { uint get(); }
struct A : IValue { uint get() { return 2; } }
struct B : IValue { uint get() { return 5; } }

[__specializePerConformance]
T changingResult<T : IValue>(T value) { return value; }

[__specializePerConformance]
uint reference<T : IValue>(inout T value) { return value.get(); }

[__specializePerConformance]
uint independent<T : IValue, U : IValue>(T a, U b) { return a.get() + b.get(); }

RWStructuredBuffer<uint> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let a = createDynamicObject<IValue>(tid.x, 0u);
    let b = createDynamicObject<IValue>(tid.y, 0u);
    outputBuffer[tid.x] = changingResult(a).get() + reference(a) + independent(a, b);
}
// CHECK-DAG: the return type changes across concrete specializations
// CHECK-DAG: type-changing reference or nested payload parameters are not supported
// CHECK-DAG: multiple independent dynamic types are not supported
