//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -capability spvShader64BitIndexingEXT

// Capability-declaration smoke test for the spvShader64BitIndexingEXT atom (issue #11538).
//
// The atom is inert (no emit wiring), so this does not exercise any atom-driven emission. Two
// independent things are checked:
//   1. The [require(spvShader64BitIndexingEXT)] clause type-checks. This is the actual regression
//      guard: if the atom were removed from slang-capabilities.capdef it becomes an
//      "unknown capability name" error and this file fails to compile.
//   2. The hand-authored OpExtension/OpCapability inside the spirv_asm block survive to emission
//      (they are hoisted verbatim, independent of the atom — a green FileCheck here is NOT evidence
//      that the atom itself drives any emission). OpCapability precedes OpExtension in SPIR-V layout.
// -capability is passed on the directive (as sibling spirv_asm tests do) so the [require] does not
// trip the implicit-capability-upgrade warning under restrictive capability checking.

RWStructuredBuffer<int> outputBuffer;

[require(spvShader64BitIndexingEXT)]
int requireShader64BitIndexing(int n)
{
    return spirv_asm
    {
        OpExtension "SPV_EXT_shader_64bit_indexing";
        OpCapability Shader64BitIndexingEXT;
        OpConstant $$int %two 2;
        OpIMul $$int result $n %two
    };
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    outputBuffer[i] = requireShader64BitIndexing(outputBuffer[i]);
}

// CHECK: OpCapability Shader64BitIndexingEXT
// CHECK: OpExtension "SPV_EXT_shader_64bit_indexing"
