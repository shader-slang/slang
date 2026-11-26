//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0 -g0
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0 -g0 -minimum-slang-optimization
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0 -g2
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0 -g2 -minimum-slang-optimization

// The vector constant is used only as an incoming phi value without debug information.
// It must still be emitted at module scope instead of leaving an undefined forward reference.
// With debug information, both the loop header and conditional merge keep their phi prefix
// immediately after the label, before DebugScope instructions.
// CHECK: %[[ONE:[A-Za-z0-9_]+]] = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
// CHECK: OpFunction
// CHECK: OpBranch %[[HEADER:[A-Za-z0-9_]+]]
// CHECK: %[[HEADER]] = OpLabel
// CHECK-NEXT: {{.*}} = OpPhi %v4float
// CHECK: OpLoopMerge
// CHECK: OpBitwiseAnd %uint
// CHECK: OpSelectionMerge %[[MERGE:[A-Za-z0-9_]+]] None
// CHECK: %[[MERGE]] = OpLabel
// CHECK-NEXT: {{.*}} = OpPhi %v4float {{.*}}%[[ONE]]

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4 value = output[0];
    for (uint i = 0; i < tid.x; ++i)
    {
        if ((i & 1) == 0)
            value = output[i];
        else
            value = float4(1.0);
        output[i + 1] = value;
    }
    output[tid.x + 1] = value;
}
