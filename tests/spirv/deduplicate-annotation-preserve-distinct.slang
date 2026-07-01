//TEST:SIMPLE(filecheck=CHECK):-emit-spirv-directly -target spirv-asm -entry computeMain -stage compute -O0

// Check that annotation instructions with distinct operands are preserved.

// CHECK: OpName %[[ONE:[A-Za-z0-9_]+]] "one"
// CHECK: OpName %[[TWO:[A-Za-z0-9_]+]] "two"
// CHECK: OpName %[[STRUCT:[A-Za-z0-9_]+]] "payloadStruct"
// CHECK: OpDecorate %[[ONE]] RelaxedPrecision
// CHECK: OpDecorate %[[TWO]] RelaxedPrecision
// CHECK: OpDecorateString %[[ONE]] UserSemantic "SEM"
// CHECK: OpDecorateString %[[ONE]] UserSemantic "SEM_ALT"
// CHECK: OpDecorateString %[[ONE]] UserTypeGOOGLE "TYPE"
// CHECK: OpDecorateString %[[TWO]] UserSemantic "SEM"
// CHECK: OpMemberDecorate %[[STRUCT]] 0 Offset 0
// CHECK: OpMemberDecorate %[[STRUCT]] 1 Offset 4

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int r = spirv_asm
    {
        OpExtension "SPV_GOOGLE_decorate_string";
        OpExtension "SPV_GOOGLE_user_type";
        OpConstant $$int %one 1;
        OpConstant $$int %two 2;
        %payloadStruct = OpTypeStruct $$int $$int;
        OpDecorate %payloadStruct Block;
        OpDecorate %one RelaxedPrecision;
        OpDecorate %two RelaxedPrecision;
        OpDecorateString %one UserSemantic "SEM";
        OpDecorateString %one UserSemantic "SEM_ALT";
        OpDecorateString %one UserTypeGOOGLE "TYPE";
        OpDecorateString %two UserSemantic "SEM";
        OpMemberDecorate %payloadStruct 0 Offset 0;
        OpMemberDecorate %payloadStruct 1 Offset 4;
        result : $$int = OpIAdd %one %two;
    };
    outputBuffer[0] = r;
}
