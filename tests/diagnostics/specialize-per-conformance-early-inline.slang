//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain

[__specializePerConformance]
[__unsafeForceInlineEarly]
uint twice<T : __BuiltinArithmeticType>(T value) { return uint(value + value); }
RWStructuredBuffer<uint> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID) { outputBuffer[tid.x] = twice(tid.x); }
// CHECK: early forced inlining removes the helper boundary
