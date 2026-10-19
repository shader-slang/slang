//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC): -target wgsl -entry genericMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC): -target hlsl -entry genericMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CONST): -target hlsl -entry constantMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CONST): -target hlsl -entry constantMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CONSTW): -target wgsl -entry constantWgslMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CONSTW): -target wgsl -entry constantWgslMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=BITCAST): -target wgsl -entry bitCastMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BITCAST): -target hlsl -entry bitCastMain -stage compute
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

// Module-scope constants (HLSL). A `nullptr` constant is folded into its uses, so it is reported
// where it is used; a struct constant that holds a pointer is reported at its initializer. WGSL
// builds such a struct where it is used, from the `nullptr` that is reported there (CONSTW, a copy of
// constantMain so that its carets sit under their own line).
static const Ptr<float> kNullPointer = nullptr;

struct PointerAndCount
{
    Ptr<float> pointer;
    uint count;
};
static const PointerAndCount kPointerAndCount = { nullptr, 3 };
//CONST:                                        ^ pointers to memory addresses are not supported on this target
//CONST:                                        ^ this uses a pointer to a memory address, which the current code generation target cannot represent

[shader("compute")]
[numthreads(1, 1, 1)]
void constantMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = (kNullPointer == kPointerAndCount.pointer) ? kPointerAndCount.count : 0;
//CONST:                                ^^ pointers to memory addresses are not supported on this target
//CONST:                                ^^ this uses a pointer to a memory address, which the current code generation target cannot represent
}

[shader("compute")]
[numthreads(1, 1, 1)]
void constantWgslMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = (kNullPointer == kPointerAndCount.pointer) ? kPointerAndCount.count : 0;
//CONSTW:                               ^^ pointers to memory addresses are not supported on this target
//CONSTW:                               ^^ this uses a pointer to a memory address, which the current code generation target cannot represent
}

// A pointer made by `bit_cast` from an integer address is diagnosed once. The cast has no
// location of its own after lowering, so it is reported at its first use.
[shader("compute")]
[numthreads(1, 1, 1)]
void bitCastMain(uint3 tid: SV_DispatchThreadID)
{
    const uint2 a = addresses[tid.x];
    float* p = bit_cast<float*>(uint64_t(a.x) | (uint64_t(a.y) << 32));
    outputBuffer[tid.x] = *p;
//BITCAST:              ^ pointers to memory addresses are not supported on this target
//BITCAST:              ^ this uses a pointer to a memory address, which the current code generation target cannot represent
}
