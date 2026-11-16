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
// a read-before-definition path must not leave the phi-elimination register
// unwritten: the exhausted-search path — the one that stores the multi-level
// break level marker (2) into the level variable when exiting the inlined
// `find` region — must also store a concrete value into the result register,
// so downstream SSA promotion does not treat the register as loop-carried.
// (The `%float_0` of the SPV check and the `0.0f` of the HLSL check are the
// same default value in each target's spelling.)

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly
//TEST:SIMPLE(filecheck=IR): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly -dump-ir-after eliminateMultiLevelBreak
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage raygeneration -entry main

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

// IR: = LoadFromUninitializedMemory
// IR-NEXT: store(

// SPV: OpStore %{{[A-Za-z0-9_]+}} %int_2
// SPV-NEXT: OpStore %{{[A-Za-z0-9_]+}} %float_0

// The exhausted-search path in textual output: after storing the break-level
// marker it also stores the concrete default into the result register.
// HLSL: _S{{[0-9]+}} = 2L;
// HLSL: _S{{[0-9]+}} = 0.0f;
