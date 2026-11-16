// Regression test for shader-slang/slang#14493.
//
// `legalizeDefUse` (called from `eliminateMultiLevelBreak`) can hoist a local
// value out of an inner control-flow region all the way to a loop header.
// When it does, the hoisted value must receive a per-iteration init: without
// one, a read on a path that skips the real definition picks up the value
// stored by a previous loop iteration, so the value is carried around the
// loop back edge. In this shader that keeps the hoisted `find` result live
// across every `TraceRay` call in the bounce loop, which after `spirv-opt`
// shows up as an extra loop-header `OpPhi` — a measured ~6% GPU-time
// regression in a production Vulkan path tracer.
//
// The trigger shape is `find`: `[ForceInline]` plus a loop whose body can
// `return` early gives `eliminateMultiLevelBreak` a value defined inside the
// loop and used past its break block. After inlining into `main`'s bounce
// loop, the hoisted value lives across the `TraceRay` call.
//
// Two artifacts of the fix are checked. First, the hoist itself must store an
// explicit uninitialized-memory value into the hoisted var each iteration
// (visible in the IR right after `eliminateMultiLevelBreak`). Second, when
// phi elimination lowers that value's merge into a SPIR-V register temp, a
// read-before-definition path must not leave the register unwritten: the
// exhausted-search path (the one that stores flag `2`) must also store a
// concrete value into the result register, so downstream SSA promotion does
// not treat the register as loop-carried and keep its content live across
// the `TraceRay` call.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly
//TEST:SIMPLE(filecheck=IR): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly -dump-ir-after eliminateMultiLevelBreak

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
