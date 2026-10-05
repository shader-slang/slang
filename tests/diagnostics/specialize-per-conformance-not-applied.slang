//DIAGNOSTIC_TEST:SIMPLE(diag=RESULT_WARNING):-target hlsl -entry computeMain -DCASE=0 -conformance A:IValue=3 -conformance B:IValue=7
//DIAGNOSTIC_TEST:SIMPLE(diag=RESULT_ERROR):-target hlsl -entry computeMain -DCASE=0 -conformance A:IValue=3 -conformance B:IValue=7 -warnings-as-errors 55219
//DIAGNOSTIC_TEST:SIMPLE(diag=REFERENCE_WARNING):-target hlsl -entry computeMain -DCASE=1 -conformance A:IValue=3 -conformance B:IValue=7
//DIAGNOSTIC_TEST:SIMPLE(diag=REFERENCE_ERROR):-target hlsl -entry computeMain -DCASE=1 -conformance A:IValue=3 -conformance B:IValue=7 -warnings-as-errors 55219
//DIAGNOSTIC_TEST:SIMPLE(diag=INDEPENDENT_WARNING):-target hlsl -entry computeMain -DCASE=2 -conformance A:IValue=3 -conformance B:IValue=7
//DIAGNOSTIC_TEST:SIMPLE(diag=INDEPENDENT_ERROR):-target hlsl -entry computeMain -DCASE=2 -conformance A:IValue=3 -conformance B:IValue=7 -warnings-as-errors 55219

interface IValue { uint get(); }
struct A : IValue { uint get() { return 2; } }
struct B : IValue { uint get() { return 5; } }

[__specializePerConformance]
T changingResult<T : IValue>(T value) { return value; }

[__specializePerConformance]
uint reference<T : IValue>(inout T value) { return value.get(); }

[__specializePerConformance]
uint independent<T : IValue, U : IValue>(T a, U b) { return a.get() + b.get(); }

// Open the existential through a by-value generic call before taking an inout
// reference. Passing the existential local directly fails front-end type checking.
uint callReference<T : IValue>(T value) { return reference(value); }

RWStructuredBuffer<uint> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let a = createDynamicObject<IValue>(tid.x, 0u);
    let b = createDynamicObject<IValue>(tid.y, 0u);
#if CASE == 0
    outputBuffer[tid.x] = changingResult(a).get();
#elif CASE == 1
    outputBuffer[tid.x] = callReference(a);
#else
    outputBuffer[tid.x] = independent(a, b);
#endif
}
//RESULT_WARNING: warning E55219
//RESULT_WARNING: the return type changes across concrete specializations
//RESULT_ERROR: error E55219
//RESULT_ERROR: the return type changes across concrete specializations
//REFERENCE_WARNING: warning E55219
//REFERENCE_WARNING: type-changing reference or nested payload parameters are not supported
//REFERENCE_ERROR: error E55219
//REFERENCE_ERROR: type-changing reference or nested payload parameters are not supported
//INDEPENDENT_WARNING: warning E55219
//INDEPENDENT_WARNING: multiple independent dynamic types are not supported
//INDEPENDENT_ERROR: error E55219
//INDEPENDENT_ERROR: multiple independent dynamic types are not supported
