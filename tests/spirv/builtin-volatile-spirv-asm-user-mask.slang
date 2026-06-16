// builtin-volatile-spirv-asm-user-mask.slang
//
// Regression test for shader-slang/slang#10528.
// Pins the user-authored-memory-operand path of the `emitSPIRVAsm`
// `OpLoad` Volatile injection. SPIR-V `OpLoad` accepts at most one
// `MemoryAccess` bitmask operand; appending an auto-injected
// `Volatile` word next to a user-supplied one would produce two
// consecutive memory-access words, which `spirv-val` rejects as
// ungrammatical. Conversely, simply suppressing auto-injection when
// the user wrote any mask would silently ship a load missing the
// spec-required `Volatile` semantics — `OpLoad ... Aligned 4`
// against a flagged builtin under `vk_mem_model` would be invalid
// SPIR-V.
//
// The fix: detect a flagged builtin pointer regardless of operand
// count, then OR `Volatile` into the user-supplied mask at emit
// time. This test exercises both forms — the named `Volatile`
// keyword and a non-Volatile mask `Aligned 4` — and asserts that
// the emitted load carries `Volatile` in both cases:
//
//   - keyword form: `OpLoad ... %V Volatile` (Volatile|Volatile
//     collapses to Volatile);
//   - `Aligned 4` form: `OpLoad ... %V Volatile|Aligned 4`
//     (Volatile bit OR'd into the user-supplied Aligned mask;
//     alignment value preserved).

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry rayGenKeyword -entry rayGenAligned -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<uint> output;

// One Input variable, no variable-level Volatile decoration under
// vk_mem_model.
// CHECK: OpDecorate %[[V:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK-NOT: OpDecorate %[[V]] Volatile

// Each entry point produces exactly one `OpLoad`. End-anchor each
// positive match (`{{$}}`) so an emit regression that doubles the
// memory-access word — producing `... Volatile Volatile` or
// `... Aligned 4 Volatile` on the same line — would fail outright.
// Bit order in the printed mask is implementation-defined; accept
// either `Volatile|Aligned` or `Aligned|Volatile`.
// CHECK-DAG: OpLoad %{{[^ ]+}} %[[V]] Volatile{{$}}
// CHECK-DAG: OpLoad %{{[^ ]+}} %[[V]] {{(Volatile\|Aligned|Aligned\|Volatile)}} 4{{$}}

uint readKeywordMask()
{
    return spirv_asm
    {
        OpCapability GroupNonUniform;
        result:$$uint = OpLoad builtin(SubgroupLocalInvocationId:uint) Volatile
    };
}

uint readAlignedMask()
{
    return spirv_asm
    {
        OpCapability GroupNonUniform;
        result:$$uint = OpLoad builtin(SubgroupLocalInvocationId:uint) Aligned 4
    };
}

[shader("raygeneration")]
void rayGenKeyword()
{
    output[0] = readKeywordMask();
}

[shader("raygeneration")]
void rayGenAligned()
{
    output[1] = readAlignedMask();
}
