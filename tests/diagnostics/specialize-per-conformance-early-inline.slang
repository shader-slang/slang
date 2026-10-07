//DIAGNOSTIC_TEST:SIMPLE(diag=WARNING):-target hlsl -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=ERROR):-target hlsl -entry computeMain -warnings-as-errors 55219

[__specializePerConformance]
[__unsafeForceInlineEarly]
T twice<T : __BuiltinArithmeticType>(T value) { return value + value; }
RWStructuredBuffer<uint> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID) { outputBuffer[tid.x] = twice(tid.x); }
//WARNING: warning E55219
//WARNING: early forced inlining removes the helper boundary
//ERROR: error E55219
//ERROR: early forced inlining removes the helper boundary
