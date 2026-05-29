//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_WARN):-target spirv-asm -entry computeMain -stage compute -DWARN
//TEST:SIMPLE(filecheck=CHECK_OK):-target spirv-asm -entry computeMain -stage compute -DNO_WARN
//TEST:SIMPLE(filecheck=CHECK_NODE_PAYLOAD_OK):-target spirv-asm -entry computeMain -stage compute -DNODE_PAYLOAD_OK -skip-spirv-validation

// Check that direct layout-sensitive OpType declarations in spirv_asm warn,
// while using Slang-typed pointer operands does not.

#if defined(WARN)
[numthreads(1, 1, 1)]
void computeMain()
{
    let value = spirv_asm
    {
        %arraySize = OpConstant $$uint 4;
        // CHECK_WARN: warning[E29116]: layout-sensitive SPIR-V type declaration in spirv_asm
        // CHECK_WARN: layout-sensitive-type-diagnostic.slang:[[# @LINE+2]]:
        // CHECK_WARN: layout-sensitive SPIR-V type declaration 'OpTypeArray' in spirv_asm may not preserve Slang data-layout information
        %array = OpTypeArray $$float %arraySize;
        // CHECK_WARN: warning[E29116]: layout-sensitive SPIR-V type declaration in spirv_asm
        // CHECK_WARN: layout-sensitive-type-diagnostic.slang:[[# @LINE+2]]:
        // CHECK_WARN: layout-sensitive SPIR-V type declaration 'OpTypeRuntimeArray' in spirv_asm may not preserve Slang data-layout information
        %runtimeArray = OpTypeRuntimeArray $$float;
        // CHECK_WARN: warning[E29116]: layout-sensitive SPIR-V type declaration in spirv_asm
        // CHECK_WARN: layout-sensitive-type-diagnostic.slang:[[# @LINE+2]]:
        // CHECK_WARN: layout-sensitive SPIR-V type declaration 'OpTypePointer' in spirv_asm may not preserve Slang data-layout information
        %pointer = OpTypePointer StorageBuffer %runtimeArray;
        result:$$uint = OpConstant 1;
    };
}
#elif defined(NO_WARN)
// CHECK_OK-NOT: layout-sensitive SPIR-V type declaration
// CHECK_OK: OpAccessChain
// CHECK_OK-NOT: layout-sensitive SPIR-V type declaration

RWStructuredBuffer<uint, Std430DataLayout> buffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let pointer = __getStructuredBufferElementPtr(buffer, 0);
    let value = spirv_asm
    {
        %unused = OpTypeBool;
        result:$$uint = OpLoad $pointer;
    };
    buffer[0] = value;
}
#elif defined(NODE_PAYLOAD_OK)
// CHECK_NODE_PAYLOAD_OK-NOT: layout-sensitive SPIR-V type declaration
// CHECK_NODE_PAYLOAD_OK: OpEntryPoint
// CHECK_NODE_PAYLOAD_OK-NOT: layout-sensitive SPIR-V type declaration

[numthreads(1, 1, 1)]
void computeMain()
{
    let value = spirv_asm
    {
        %nodePayload = OpTypeNodePayloadArrayAMDX $$uint;
        %nodePayloadPtr = OpTypePointer NodePayloadAMDX %nodePayload;
        result:$$uint = OpConstant 1;
    };
}
#endif
