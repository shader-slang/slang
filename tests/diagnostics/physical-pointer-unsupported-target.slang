// https://github.com/shader-slang/slang/issues/13632
//
// HLSL, DXIL and WGSL have no pointers to memory addresses, so a `T*` value that shader code
// makes from an integer, with `bit_cast`, or from `nullptr` is an error on those targets. Each
// such value is reported once, where it is made or where `nullptr` is used, at every optimization
// level. A value that only debug information uses is not emitted and is not reported. SPIR-V
// compiles the same shader.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target dxil -entry computeMain -profile cs_6_5
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target dxil -entry computeMain -profile cs_6_5 -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -g
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute -g -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=WGSL_STATIC): -target wgsl -entry staticMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=WGSL_STATIC): -target wgsl -entry staticMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=D3D_STATIC): -target hlsl -entry staticMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=D3D_STATIC): -target hlsl -entry staticMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=D3D_STATIC): -target dxil -entry staticMain -profile cs_6_5
//DIAGNOSTIC_TEST:SIMPLE(diag=CONST): -target wgsl -entry constantMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CONST): -target wgsl -entry constantMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CONST): -target hlsl -entry constantMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CONST): -target hlsl -entry constantMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CONST): -target dxil -entry constantMain -profile cs_6_5
//DIAGNOSTIC_TEST:SIMPLE(diag=CONSTEXPR): -target wgsl -entry constexprMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CONSTEXPR): -target hlsl -entry constexprMain -stage compute -O0
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

// SPV: OpConvertUToPtr

RWStructuredBuffer<uint2> addrs;
RWStructuredBuffer<float> outBuf;

// Both specializations share the cast's location, so it is reported once.
T* pointerAt<T>(uint2 a)
{
    return (T*)(uint64_t(a.x) | (uint64_t(a.y) << 32));
//CHECK:   ^ pointers to memory addresses are not supported on this target
//CHECK:   ^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent
}

bool isSet(float* p)
{
    return p != nullptr;
//CHECK:     ^^ pointers to memory addresses are not supported on this target
//CHECK:     ^^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    const uint2 a = addrs[tid.x];
    // Only debug information uses `unused`, so it is not reported.
    float* unused = (float*)uint64_t(a.y);
    float* p = pointerAt<float>(a);
    int* q = pointerAt<int>(a);
    if (q)
//CHECK:^ pointers to memory addresses are not supported on this target
//CHECK:^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent
        outBuf[tid.x] = 2.0;
    // `bit_cast` has no location of its own, so its pointer is reported where it is first used.
    float* r = bit_cast<float*>(uint64_t(a.x));
    outBuf[tid.x] = isSet(p) ? *r : 0.0;
//CHECK:                     ^ pointers to memory addresses are not supported on this target
//CHECK:                     ^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent
}

// HLSL keeps this initializer at module scope, so it is reported at the declaration. WGSL moves
// it into the entry point, so it is reported where `gPointer` is read.
static float* gPointer = nullptr;
//D3D_STATIC: ^^^^^^^^ pointers to memory addresses are not supported on this target
//D3D_STATIC: ^^^^^^^^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent

[shader("compute")]
[numthreads(1, 1, 1)]
void staticMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = (uint64_t(gPointer) != 0) ? 1.0 : 0.0;
//WGSL_STATIC:               ^ pointers to memory addresses are not supported on this target
//WGSL_STATIC:               ^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent
}

struct PointerAndCount
{
    float* p;
    int n;
};

static const PointerAndCount kPointerAndCount = { nullptr, 3 };
//CONST:                                        ^ pointers to memory addresses are not supported on this target
//CONST:                                        ^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent

[shader("compute")]
[numthreads(1, 1, 1)]
void constantMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = (uint64_t(kPointerAndCount.p) != 0) ? 1.0 : float(kPointerAndCount.n);
}

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> colors;

// A texel offset must be a compile-time constant, so this cast is a `@ConstExpr` value.
[shader("compute")]
[numthreads(1, 1, 1)]
void constexprMain(uint3 tid: SV_DispatchThreadID)
{
    float* p = (float*)16ULL;
//CONSTEXPR:   ^ pointers to memory addresses are not supported on this target
//CONSTEXPR:   ^ this creates or uses a pointer to a memory address, which the current code generation target cannot represent
    colors[tid.x] = tex.SampleLevel(samp, float2(0.5), 0, int2(int(uint64_t(p) >> 3), 0));
}
