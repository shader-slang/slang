// builtin-volatile-vk-mem-model.slang
//
// Regression test for shader-slang/slang#10528.
// Under the Vulkan memory model the variable-level `OpDecorate Volatile`
// is invalid; the corresponding `Volatile` semantics for subgroup
// builtins in raytracing stages must instead be expressed as the
// `Volatile` memory access mask on each `OpLoad`.
//
// Two raygeneration entry points each independently read
// `WaveGetLaneIndex()`. After [ForceInline] both end up in the same
// SPIR-V module and (with the current IR pipeline) share a single
// `OpVariable` for `SubgroupLocalInvocationId`. Each entry point
// emits its own `OpLoad` that must carry the `Volatile` memory
// access mask. The pre-cache `m_volatileBuiltinGlobalVars.add()` in
// `getBuiltinGlobalVar` ensures both call sites' IR-side operands
// are flagged so the per-access mask injection in `emitSPIRVAsm`
// fires for every `OpLoad`, not just the first one.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry rayGenMainA -entry rayGenMainB -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<uint> output;

// SPIR-V layout: Annotations come before Types/Variables. Match the
// `OpDecorate ... BuiltIn` line first, capture the variable name, and
// pin the absence of any `OpDecorate %V Volatile` between annotations
// (variable-level `Volatile` is invalid under the Vulkan memory model).
// CHECK: OpDecorate %[[V:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK-NOT: OpDecorate %[[V]] Volatile

// Then the variable declaration in the Types/Variables section.
// CHECK: %[[V]] = OpVariable {{.*}} Input

// And both entry points emit an `OpLoad` of `%V` carrying the
// `Volatile` memory access mask. The two-count assertion catches
// the cache-hit alias case directly: if the second call site
// missed the `m_volatileBuiltinGlobalVars` set, only one of the
// two loads would carry the mask.
// CHECK-COUNT-2: OpLoad {{.*}} %[[V]] {{.*}}Volatile

[shader("raygeneration")]
void rayGenMainA()
{
    output[0] = WaveGetLaneIndex();
}

[shader("raygeneration")]
void rayGenMainB()
{
    output[1] = WaveGetLaneIndex();
}
