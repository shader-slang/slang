// Regression test for shader-slang/slang#14493.
//
// `legalizeDefUse` (called from `eliminateMultiLevelBreak`) can hoist a local
// value out of an inner control-flow region all the way to a loop header.
// When it does, the hoisted value must receive an init at every entry of the
// loop whose header it lands in: without one, a read on a path that skips the
// real definition picks up a value stored on an earlier entry, so the value
// is carried around the loop back edge. In this shader that keeps the hoisted
// `find` result live across every `TraceRay` call in the bounce loop, which
// after `spirv-opt` shows up as an extra loop-header `OpPhi` — a measured ~6%
// GPU-time regression in a production Vulkan path tracer.
//
// The trigger shape is `find`: `[ForceInline]` plus a loop whose body can
// `return` early gives `eliminateMultiLevelBreak` a value defined inside the
// loop and used past its break block. After inlining into `main`'s bounce
// loop, the hoisted value lives across the `TraceRay` call.
//
// Artifacts of the fix are checked at three layers. In the IR right after
// `eliminateMultiLevelBreak`, the hoist must store an explicit
// uninitialized-memory value into the hoisted var. In emitted SPIR-V and HLSL,
// the exhausted-search path — the one that stores the multi-level break level
// marker (2) into the level variable when exiting the inlined region — must
// not leave the phi-elimination result register unwritten: the level marker
// store and a concrete default-value store both appear on that path, so
// downstream SSA promotion does not treat the register as loop-carried.
// (`%float_0` and `0.0f` are the same default in each target's spelling.)
//
// The `mainVec` and `mainMat` variants exercise the vector and matrix arms of
// the default-value materialization (a scalar `float` alone would not catch a
// regression in default construction for those types, since the store silently
// falls back to the old unwritten-register behavior).

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly
//TEST:SIMPLE(filecheck=IR): -target spirv-asm -stage raygeneration -entry main -dump-ir-after eliminateMultiLevelBreak
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage raygeneration -entry main
//TEST:SIMPLE(filecheck=SPVVEC): -target spirv-asm -stage raygeneration -entry mainVec -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSLVEC): -target hlsl -stage raygeneration -entry mainVec
//TEST:SIMPLE(filecheck=SPVMAT): -target spirv-asm -stage raygeneration -entry mainMat -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSLMAT): -target hlsl -stage raygeneration -entry mainMat

RaytracingAccelerationStructure scene;
RWStructuredBuffer<float> buf;

[ForceInline]
float find(float u)
{
    for (uint i = 0; i < 4; ++i)
        if (u < buf[i])
            return buf[i];
    return 1.0;
}

[ForceInline]
float4 findVec(float u)
{
    for (uint i = 0; i < 4; ++i)
        if (u < buf[i])
            return buf[i].xxxx;
    return float4(1.0);
}

[ForceInline]
float2x2 findMat(float u)
{
    for (uint i = 0; i < 4; ++i)
        if (u < buf[i])
            return float2x2(buf[i], 0, 0, buf[i]);
    return float2x2(1, 0, 0, 1);
}

[shader("raygeneration")]
void main()
{
    // The flat initializer values do not matter for this compile-only test;
    // under flattened brace-initialization they do not land where they appear
    // to (1e30 goes to TMin, not TMax). Kept verbatim from the issue report.
    RayDesc ray = { 0, 0, 1, 1e30 };
    float t = 0;
    for (int k = 0; k < 8; ++k)
    {
        TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, t);
        ray.TMax = find(t);
    }
    buf[0] = ray.TMax;
}

[shader("raygeneration")]
void mainVec()
{
    RayDesc ray = { 0, 0, 1, 1e30 };
    float t = 0;
    for (int k = 0; k < 8; ++k)
    {
        TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, t);
        ray.TMax = findVec(t).x;
    }
    buf[0] = ray.TMax;
}

[shader("raygeneration")]
void mainMat()
{
    RayDesc ray = { 0, 0, 1, 1e30 };
    float t = 0;
    for (int k = 0; k < 8; ++k)
    {
        TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, t);
        ray.TMax = findMat(t)[0][0];
    }
    buf[0] = ray.TMax;
}

// IR: ### AFTER eliminateMultiLevelBreak:
// IR: = LoadFromUninitializedMemory
// IR-NEXT: store(

// SPV: %[[F0:[A-Za-z0-9_]+]] = OpConstant %float 0
// SPV: OpStore %{{[A-Za-z0-9_]+}} %int_2
// SPV-NEXT: OpStore %{{[A-Za-z0-9_]+}} %[[F0]]

// The exhausted-search path in textual output: both the break-level marker
// and the concrete default store appear on that path.
// HLSL: _S{{[0-9]+}} = 2L;
// HLSL: _S{{[0-9]+}} = 0.0f;

// SPVVEC: %[[V0:[A-Za-z0-9_]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// SPVVEC: OpStore %{{[A-Za-z0-9_]+}} %int_2
// SPVVEC-NEXT: OpStore %{{[A-Za-z0-9_]+}} %[[V0]]

// HLSLVEC: _S{{[0-9]+}} = 2L;
// HLSLVEC: _S{{[0-9]+}} = (float4)0.0f;

// SPVMAT: %[[M0:[A-Za-z0-9_]+]] = OpConstantComposite %mat2v2float
// SPVMAT: OpStore %{{[A-Za-z0-9_]+}} %int_2
// SPVMAT-NEXT: OpStore %{{[A-Za-z0-9_]+}} %[[M0]]

// HLSLMAT: _S{{[0-9]+}} = 2L;
// HLSLMAT: _S{{[0-9]+}} = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
