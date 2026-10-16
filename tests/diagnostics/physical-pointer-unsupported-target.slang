//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC): -target wgsl -entry genericMain -stage compute

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
//CHECK:       ^ this creates a pointer from an address, reads one from memory or uses 'nullptr', but the current code generation target has no pointers to memory addresses
    outputBuffer[tid.x] = (p != nullptr) ? 1.0 : 0.0;
//CHECK:                     ^^ pointers to memory addresses are not supported on this target
//CHECK:                     ^^ this creates a pointer from an address, reads one from memory or uses 'nullptr', but the current code generation target has no pointers to memory addresses
}

// The same inside a generic function.
T* pointerAt<T>(uint2 address)
{
    return (T*)(uint64_t(address.x) | (uint64_t(address.y) << 32));
//GENERIC: ^ pointers to memory addresses are not supported on this target
//GENERIC: ^ this creates a pointer from an address, reads one from memory or uses 'nullptr', but the current code generation target has no pointers to memory addresses
}

[shader("compute")]
[numthreads(1, 1, 1)]
void genericMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = (pointerAt<float>(addresses[tid.x]) != nullptr) ? 1.0 : 0.0;
//GENERIC:                                                    ^^ pointers to memory addresses are not supported on this target
//GENERIC:                                                    ^^ this creates a pointer from an address, reads one from memory or uses 'nullptr', but the current code generation target has no pointers to memory addresses
}
