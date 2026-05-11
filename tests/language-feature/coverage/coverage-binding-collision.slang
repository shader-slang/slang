// Verifies that an explicit `-trace-coverage-binding` colliding with
// an existing user parameter produces a Slang-level error anchored
// at the user declaration, rather than letting the cryptic
// downstream "duplicate register" / spirv-val "two params at the
// same DescriptorSet/Binding" error surface.
//
// The user binds `userBuf` at `register(u3, space0)`, and we pin
// `__slang_coverage` at the same `(register, space)` via
// `-trace-coverage-binding 3 0`. The IR coverage pass must diagnose
// the collision before synthesizing the second parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -trace-coverage -trace-coverage-binding 3 0 -profile cs_5_0

RWStructuredBuffer<uint> userBuf : register(u3, space0);
//CHECK:                 ^^^^^^^ error E45101

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    userBuf[0] = 1;
}
