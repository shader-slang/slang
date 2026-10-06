//DIAGNOSTIC_TEST:SIMPLE(diag=PACK0_WARNING):-target hlsl -entry computeMain -conformance A:IValue=0 -conformance B:IValue=1 -DPACK_SIZE=0
//DIAGNOSTIC_TEST:SIMPLE(diag=PACK0_ERROR):-target hlsl -entry computeMain -conformance A:IValue=0 -conformance B:IValue=1 -DPACK_SIZE=0 -warnings-as-errors 55219
//DIAGNOSTIC_TEST:SIMPLE(diag=PACK1_WARNING):-target hlsl -entry computeMain -conformance A:IValue=0 -conformance B:IValue=1 -DPACK_SIZE=1
//DIAGNOSTIC_TEST:SIMPLE(diag=PACK1_ERROR):-target hlsl -entry computeMain -conformance A:IValue=0 -conformance B:IValue=1 -DPACK_SIZE=1 -warnings-as-errors 55219

interface IValue { uint get(); }
struct A : IValue { uint get() { return 2; } }
struct B : IValue { uint get() { return 5; } }

// Specialization expands the empty pack away and unwraps the singleton pack.
// Compare the final signature with the dispatcher so these valid requests warn
// and fall back instead of asserting equality with the unexpanded type.
[__specializePerConformance]
uint helper<T : IValue, each U>(T value, expand each U args)
{
    return value.get();
}

RWStructuredBuffer<uint> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let value = createDynamicObject<IValue>(tid.x, 0u);
#if PACK_SIZE == 0
    outputBuffer[tid.x] = helper(value);
#else
    outputBuffer[tid.x] = helper(value, tid.y);
#endif
}
//PACK0_WARNING: warning E55219
//PACK0_WARNING: concrete specialization changes the parameter count
//PACK0_ERROR: error E55219
//PACK0_ERROR: concrete specialization changes the parameter count
//PACK1_WARNING: warning E55219
//PACK1_WARNING: type-changing reference or nested payload parameters are not supported
//PACK1_ERROR: error E55219
//PACK1_ERROR: type-changing reference or nested payload parameters are not supported
