// `OpExecutionModeId __entryPoint ...` in `spirv_asm` applies the execution mode to every
// entry point that calls the enclosing function, as `OpExecutionMode __entryPoint` does.
// Its extra operands are ids: here a specialization constant and a constant defined in the
// same block. See https://github.com/shader-slang/slang/issues/13438.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_6
//TEST:SIMPLE(filecheck=COUNT): -target spirv -profile spirv_1_6

// CHECK-DAG: OpCapability RayTracingOpacityMicromapExecutionModeKHR
// CHECK-DAG: OpExtension "SPV_KHR_opacity_micromap"
// CHECK-DAG: OpEntryPoint GLCompute %[[A:[A-Za-z0-9_]+]] "computeA"
// CHECK-DAG: OpEntryPoint GLCompute %[[B:[A-Za-z0-9_]+]] "computeB"
// CHECK-DAG: OpEntryPoint GLCompute %[[C:[A-Za-z0-9_]+]] "computeC"
// CHECK-DAG: OpExecutionModeId %[[A]] OpacityMicromapIdKHR %[[SPEC:[A-Za-z0-9_]+]]
// CHECK-DAG: OpExecutionModeId %[[B]] OpacityMicromapIdKHR %[[SPEC]]
// CHECK-DAG: OpExecutionModeId %[[C]] OpacityMicromapIdKHR %[[TRUE:[A-Za-z0-9_]+]]
// CHECK-DAG: OpDecorate %[[SPEC]] SpecId 2
// CHECK-DAG: %[[SPEC]] = OpSpecConstantFalse %bool
// CHECK-DAG: %[[TRUE]] = OpConstantTrue %bool

// Each entry point gets the mode once, even when the helper is called twice.
// COUNT-COUNT-3: OpExecutionModeId
// COUNT-NOT: OpExecutionModeId

[vk::constant_id(2)]
const bool kEnableOpacityMicromap = false;

RWStructuredBuffer<uint> outputBuffer;

// Shared helper: the execution mode lands on whichever entry points call it.
void enableOpacityMicromapFromSpecConstant()
{
    spirv_asm
    {
        OpCapability RayTracingOpacityMicromapExecutionModeKHR;
        OpExtension "SPV_KHR_opacity_micromap";
        OpExecutionModeId __entryPoint OpacityMicromapIdKHR $kEnableOpacityMicromap
    };
}

void enableOpacityMicromap()
{
    spirv_asm
    {
        OpCapability RayTracingOpacityMicromapExecutionModeKHR;
        OpExtension "SPV_KHR_opacity_micromap";
        %enable = OpConstantTrue $$bool;
        OpExecutionModeId __entryPoint OpacityMicromapIdKHR %enable
    };
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeA(uint3 tid: SV_DispatchThreadID)
{
    enableOpacityMicromapFromSpecConstant();
    enableOpacityMicromapFromSpecConstant();
    outputBuffer[tid.x] = 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeB(uint3 tid: SV_DispatchThreadID)
{
    enableOpacityMicromapFromSpecConstant();
    outputBuffer[tid.x] = 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeC(uint3 tid: SV_DispatchThreadID)
{
    enableOpacityMicromap();
    outputBuffer[tid.x] = 3;
}
