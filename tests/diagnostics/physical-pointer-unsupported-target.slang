// https://github.com/shader-slang/slang/issues/13632
//
// HLSL, DXIL and WGSL have no pointers to memory addresses, so a `T*` value that shader code
// makes from an integer, with `bit_cast`, or from `nullptr` is an error on those targets. Each
// such value is reported once per source location, at every optimization level: at the cast or
// the `nullptr` use when it has a location, otherwise at a use, and without a location as a last
// resort. A value that only debug information observes is removed instead, so `-g` reports and
// emits the same as a build without it. SPIR-V compiles the same shader.
//
// `computeMain` runs on every target and optimization level. Each other entry point tests one
// shape, with a few target and optimization combinations.

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
//DIAGNOSTIC_TEST:SIMPLE(diag=DEBUG_CALL): -target hlsl -entry debugCallMain -stage compute -g
//DIAGNOSTIC_TEST:SIMPLE(diag=DEBUG_CALL): -target wgsl -entry debugCallMain -stage compute -g -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=NOLOC): -target wgsl -entry reinterpretMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NOLOC): -target hlsl -entry reinterpretMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -minimum-slang-optimization
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute -minimum-slang-optimization
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=DEBUG_ONLY): -target hlsl -entry debugOnlyMain -stage compute -g
//TEST:SIMPLE(filecheck=DEBUG_ONLY): -target hlsl -entry debugOnlyMain -stage compute -g -O0
//TEST:SIMPLE(filecheck=DEBUG_ONLY): -target wgsl -entry debugOnlyMain -stage compute -g
//TEST:SIMPLE(filecheck=DEBUG_ONLY): -target wgsl -entry debugOnlyMain -stage compute -g -O0

// SPV: OpConvertUToPtr
// DEBUG_ONLY-NOT: Ptr<
// DEBUG_ONLY-NOT: ptr<,
// DEBUG_ONLY: debugOnlyMain
// DEBUG_ONLY-NOT: Ptr<
// DEBUG_ONLY-NOT: ptr<,

RWStructuredBuffer<uint2> addrs;
RWStructuredBuffer<float> outBuf;

// Both specializations share the cast's location, so it is reported once.
T* pointerAt<T>(uint2 a)
{
    return (T*)(uint64_t(a.x) | (uint64_t(a.y) << 32));
//CHECK:   ^ pointers to memory addresses are not supported on this target
//CHECK:   ^ the current code generation target has no pointers to memory addresses
}

bool isSet(float* p)
{
    return p != nullptr;
//CHECK:     ^^ pointers to memory addresses are not supported on this target
//CHECK:     ^^ the current code generation target has no pointers to memory addresses
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
//CHECK:^ the current code generation target has no pointers to memory addresses
        outBuf[tid.x] = 2.0;
    // The `bitCast` that `bit_cast` lowers to has no location, so its pointer is reported at the
    // location of a use. Here the load `*r` takes the location of the `?:` expression.
    float* r = bit_cast<float*>(uint64_t(a.x));
    outBuf[tid.x] = isSet(p) ? *r : 0.0;
//CHECK:                     ^ pointers to memory addresses are not supported on this target
//CHECK:                     ^ the current code generation target has no pointers to memory addresses
}

// HLSL keeps this initializer at module scope, so it is reported at the declaration. WGSL moves
// it into the entry point, so it is reported where `gPointer` is read.
static float* gPointer = nullptr;
//D3D_STATIC: ^^^^^^^^ pointers to memory addresses are not supported on this target
//D3D_STATIC: ^^^^^^^^ the current code generation target has no pointers to memory addresses

[shader("compute")]
[numthreads(1, 1, 1)]
void staticMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = (uint64_t(gPointer) != 0) ? 1.0 : 0.0;
//WGSL_STATIC:               ^ pointers to memory addresses are not supported on this target
//WGSL_STATIC:               ^ the current code generation target has no pointers to memory addresses
}

struct PointerAndCount
{
    float* p;
    int n;
};

// The WGSL `CONST` runs also check that `legalizeCall` passes this `nullptr` argument by value,
// which keeps its location.
static const PointerAndCount kPointerAndCount = { nullptr, 3 };
//CONST:                                        ^ pointers to memory addresses are not supported on this target
//CONST:                                        ^ the current code generation target has no pointers to memory addresses

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
//CONSTEXPR:   ^ the current code generation target has no pointers to memory addresses
    colors[tid.x] = tex.SampleLevel(samp, float2(0.5), 0, int2(int(uint64_t(p) >> 3), 0));
}

[noinline]
bool storeAndTest(float* p)
{
    outBuf[0] = 2.0;
    return uint64_t(p) != 0;
}

// With `-g`, only debug information reads `b`, but the call has side effects and is emitted with
// its `nullptr` argument, so it is reported.
[shader("compute")]
[numthreads(1, 1, 1)]
void debugCallMain(uint3 tid: SV_DispatchThreadID)
{
    bool b = storeAndTest(nullptr);
//DEBUG_CALL:            ^ pointers to memory addresses are not supported on this target
//DEBUG_CALL:            ^ the current code generation target has no pointers to memory addresses
    outBuf[tid.x] = 1.0;
}

// `reinterpret` makes its pointer inside a compiler-generated unpacking function, so no
// instruction that creates or uses it has a location. The error is still reported, without one.
[shader("compute")]
[numthreads(1, 1, 1)]
void reinterpretMain(uint3 tid: SV_DispatchThreadID)
{
    float* p = reinterpret<float*>(addrs[tid.x]);
    outBuf[tid.x] = *p;
}
//NOLOC: pointers to memory addresses are not supported on this target
//NOLOC: the current code generation target has no pointers to memory addresses

uniform float* gUniformPointer;
static const bool kIsNull = (float*)0 == nullptr;

// Only debug information reads these values. A build without `-g` removes them as dead code, so
// with `-g` they are removed too, rather than reported or emitted.
[shader("compute")]
[numthreads(1, 1, 1)]
void debugOnlyMain(uint3 tid: SV_DispatchThreadID)
{
    uint64_t x = uint64_t((float*)uint64_t(addrs[tid.x].x));
    x = x * x;
    bool isNull = gUniformPointer == nullptr;
    bool copy = isNull;
    let isNullToo = gUniformPointer == nullptr;
    let constant = kIsNull;
    outBuf[tid.x] = 1.0;
}
