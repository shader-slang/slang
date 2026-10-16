//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC): -target wgsl -entry genericMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC): -target hlsl -entry genericMain -stage compute
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

// WGSL and HLSL have no pointers to memory addresses. A pointer cast from an integer address and a
// comparison with `nullptr` must be diagnosed, not abort the emitter (#13632).

RWStructuredBuffer<uint2> addresses;
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    const uint2 a = addresses[tid.x];
    float* p = (float*)(uint64_t(a.x) | (uint64_t(a.y) << 32));
//CHECK:       ^ pointers to memory addresses are not supported on this target
//CHECK:       ^ this uses a pointer to a memory address, which the current code generation target cannot represent
    outputBuffer[tid.x] = (p != nullptr) ? 1.0 : 0.0;
//CHECK:                     ^^ pointers to memory addresses are not supported on this target
//CHECK:                     ^^ this uses a pointer to a memory address, which the current code generation target cannot represent
}

// SPIR-V has physical storage buffer pointers, so nothing is diagnosed there.
// SPV: OpConvertUToPtr

// The same inside a generic function. Its two specializations share the cast's source location,
// which is reported once.
T* pointerAt<T>(uint2 address)
{
    return (T*)(uint64_t(address.x) | (uint64_t(address.y) << 32));
//GENERIC: ^ pointers to memory addresses are not supported on this target
//GENERIC: ^ this uses a pointer to a memory address, which the current code generation target cannot represent
}

[shader("compute")]
[numthreads(1, 1, 1)]
void genericMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = (pointerAt<float>(addresses[tid.x]) != nullptr) ? 1.0 : 0.0;
//GENERIC:                                                    ^^ pointers to memory addresses are not supported on this target
//GENERIC:                                                    ^^ this uses a pointer to a memory address, which the current code generation target cannot represent
    outputBuffer[tid.x + 1] = (pointerAt<int>(addresses[tid.x]) != nullptr) ? 1.0 : 0.0;
//GENERIC:                                                      ^^ pointers to memory addresses are not supported on this target
//GENERIC:                                                      ^^ this uses a pointer to a memory address, which the current code generation target cannot represent
}
