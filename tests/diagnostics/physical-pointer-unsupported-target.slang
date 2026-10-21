// https://github.com/shader-slang/slang/issues/13632
//
// HLSL, DXIL and WGSL have no pointers to memory addresses. A `T*` value created in shader code,
// by an integer-to-pointer cast or by `nullptr`, used to abort the emitter (E99999) or produce
// invalid `Ptr<...>` / `ptr<, ...>` output. Each such value is now reported once, where it is made
// or where `nullptr` is used, including at `-O0`. SPIR-V keeps compiling the same shader.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target dxil -entry computeMain -profile cs_6_5
//DIAGNOSTIC_TEST:SIMPLE(diag=GLOBAL): -target wgsl -entry globalMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GLOBAL): -target hlsl -entry globalMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=GLOBAL): -target dxil -entry globalMain -profile cs_6_5
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

// SPV: OpConvertUToPtr

RWStructuredBuffer<uint2> addrs;
RWStructuredBuffer<float> outBuf;

// Both specializations share the cast's location, so it is reported once.
T* pointerAt<T>(uint2 a)
{
    return (T*)(uint64_t(a.x) | (uint64_t(a.y) << 32));
//CHECK:       ^ pointers to memory addresses are not supported on this target
//CHECK:       ^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent
}

bool isSet(float* p)
{
    return p != nullptr;
//CHECK:         ^ pointers to memory addresses are not supported on this target
//CHECK:         ^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    const uint2 a = addrs[tid.x];
    float* p = pointerAt<float>(a);
    int* q = pointerAt<int>(a);
    if (q)
//CHECK: ^ pointers to memory addresses are not supported on this target
//CHECK: ^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent
        outBuf[tid.x] = 2.0;
    float* r = bit_cast<float*>(uint64_t(a.x));
//CHECK:     ^ pointers to memory addresses are not supported on this target
//CHECK:     ^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent
    outBuf[tid.x] = isSet(p) ? *r : 0.0;
}

struct PointerAndCount
{
    float* p;
    int n;
};

static float* gPointer = nullptr;
static const PointerAndCount kPointerAndCount = { nullptr, 3 };

[shader("compute")]
[numthreads(1, 1, 1)]
void globalMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = (uint64_t(gPointer) != 0 || uint64_t(kPointerAndCount.p) != 0) ? 1.0 : 0.0;
}
