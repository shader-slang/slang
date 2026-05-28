//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_WARN):-target spirv-asm -entry rayGenMain -stage raygeneration -emit-spirv-directly -capability vk_mem_model -skip-spirv-validation -DWARN
//TEST:SIMPLE(filecheck=CHECK_OK):-target spirv-asm -entry rayGenMain -stage raygeneration -emit-spirv-directly -capability vk_mem_model -DNO_WARN

// Regression test for shader-slang/slang#10528 — diagnose user-supplied
// non-literal memory-access mask on `OpLoad` of a Volatile-required
// builtin in `spirv_asm`. The auto-injection path can only OR
// compile-time literal/enum bits into the mask; an `Id`-form (or other
// non-literal kind) is not representable in `OpLoad`'s memory-access
// slot per SPIR-V grammar (the spec mandates a literal integer there).
// In the WARN variant the user supplies an `Id` mask and we expect
// `E29117` to fire. In the NO_WARN variant the user supplies a literal,
// the merge succeeds, and no warning is emitted.

RWStructuredBuffer<uint> output;

#if defined(WARN)
uint readWithIdMask()
{
    return spirv_asm
    {
        OpCapability GroupNonUniform;
        // The mask operand is an Id-form (`%runtimeMask`), not a
        // Literal or Enum. The auto-inject path can't merge it.
        // The diagnostic anchors on the Id definition (Slang's IR
        // representation of `SPIRVAsmOperandId` carries the
        // definition-site source loc, not the use site).
        // CHECK_WARN: warning[E29117]: memory-access mask on `OpLoad` of a Volatile-required builtin
        %runtimeMask = OpConstant $$uint 4;
        result:$$uint = OpLoad builtin(SubgroupLocalInvocationId:uint) %runtimeMask
    };
}

[shader("raygeneration")]
void rayGenMain()
{
    output[0] = readWithIdMask();
}
#elif defined(NO_WARN)
// CHECK_OK-NOT: warning[E29117]
// CHECK_OK-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK_OK-DAG: OpLoad {{.*}} %{{[A-Za-z0-9_]+}} {{.*}}Volatile

uint readWithLiteralMask()
{
    return spirv_asm
    {
        OpCapability GroupNonUniform;
        // Literal mask — merge succeeds, no warning, emitted as
        // `Volatile|Aligned 4`.
        result:$$uint = OpLoad builtin(SubgroupLocalInvocationId:uint) Aligned 4
    };
}

[shader("raygeneration")]
void rayGenMain()
{
    output[0] = readWithLiteralMask();
}
#endif
