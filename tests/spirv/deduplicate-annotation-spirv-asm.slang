//TEST:SIMPLE(filecheck=CHECK):-emit-spirv-directly -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT -O0

// Check that exact duplicate annotation instructions emitted from spirv_asm
// appear only once in the final SPIR-V assembly.

// CHECK: OpName %[[SCOPE:[A-Za-z0-9_]+]] "scope"
// CHECK: OpName %[[ONE:[A-Za-z0-9_]+]] "one"
// CHECK: OpName %[[FOUR:[A-Za-z0-9_]+]] "four"
// CHECK: OpName %[[STRUCT:[A-Za-z0-9_]+]] "payloadStruct"
// CHECK: OpName %[[GROUP:[A-Za-z0-9_]+]] "decorGroup"
// CHECK: OpName %[[GROUP_B:[A-Za-z0-9_]+]] "decorGroupB"
// CHECK: %[[GROUP]] = OpDecorationGroup
// CHECK: %[[GROUP_B]] = OpDecorationGroup
// CHECK-NOT: OpFunction
// CHECK: OpDecorate %[[ONE]] RelaxedPrecision
// CHECK-NOT: OpDecorate %[[ONE]] RelaxedPrecision
// CHECK: OpDecorateString %[[ONE]] UserSemantic "SEM"
// CHECK-NOT: OpDecorateString %[[ONE]] UserSemantic "SEM"
// CHECK: OpDecorateId %[[ONE]] UniformId %[[SCOPE]]
// CHECK-NOT: OpDecorateId %[[ONE]] UniformId %[[SCOPE]]
// CHECK: OpMemberDecorateIdEXT %[[STRUCT]] 0 OffsetIdEXT %[[FOUR]]
// CHECK-NOT: OpMemberDecorateIdEXT %[[STRUCT]] 0 OffsetIdEXT %[[FOUR]]
// CHECK: OpMemberDecorate %[[STRUCT]] 1 Offset 0
// CHECK-NOT: OpMemberDecorate %[[STRUCT]] 1 Offset 0
// CHECK: OpMemberDecorateString %[[STRUCT]] 1 UserSemantic "MEMSEM"
// CHECK-NOT: OpMemberDecorateString %[[STRUCT]] 1 UserSemantic "MEMSEM"
// CHECK: OpGroupDecorate %[[GROUP]] %[[ONE]]
// CHECK-NOT: OpGroupDecorate %[[GROUP]] %[[ONE]]
// CHECK: OpGroupDecorate %[[GROUP_B]] %[[SCOPE]]
// CHECK: OpGroupMemberDecorate %[[GROUP]] %[[STRUCT]] 1
// CHECK-NOT: OpGroupMemberDecorate %[[GROUP]] %[[STRUCT]] 1

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int r = spirv_asm
    {
        OpCapability DescriptorHeapEXT;
        OpExtension "SPV_EXT_descriptor_heap";
        OpExtension "SPV_GOOGLE_decorate_string";
        OpConstant $$int %scope 3;
        OpConstant $$int %one 1;
        OpConstant $$int %four 4;
        %bufferType = OpTypeBufferEXT Uniform;
        %payloadStruct = OpTypeStruct %bufferType $$int;
        %decorGroup = OpDecorationGroup;
        %decorGroupB = OpDecorationGroup;
        OpDecorate %one RelaxedPrecision;
        OpDecorate %one RelaxedPrecision;
        OpDecorateString %one UserSemantic "SEM";
        OpDecorateString %one UserSemantic "SEM";
        OpDecorateId %one UniformId %scope;
        OpDecorateId %one UniformId %scope;
        OpDecorate %payloadStruct Block;
        OpMemberDecorateIdEXT %payloadStruct 0 OffsetIdEXT %four;
        OpMemberDecorateIdEXT %payloadStruct 0 OffsetIdEXT %four;
        OpMemberDecorate %payloadStruct 1 Offset 0;
        OpMemberDecorate %payloadStruct 1 Offset 0;
        OpMemberDecorateString %payloadStruct 1 UserSemantic "MEMSEM";
        OpMemberDecorateString %payloadStruct 1 UserSemantic "MEMSEM";
        OpDecorate %decorGroup RelaxedPrecision;
        OpDecorate %decorGroupB RelaxedPrecision;
        OpGroupDecorate %decorGroup %one;
        OpGroupDecorate %decorGroup %one;
        OpGroupDecorate %decorGroupB %scope;
        OpGroupMemberDecorate %decorGroup %payloadStruct 1;
        OpGroupMemberDecorate %decorGroup %payloadStruct 1;
        result : $$int = OpIAdd %one %scope;
    };
    outputBuffer[0] = r;
}
