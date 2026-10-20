// `OpExecutionModeId __entryPoint ...` in `spirv_asm` applies the execution mode to every
// entry point that calls the enclosing function, as `OpExecutionMode __entryPoint` does.
// Its extra operands are ids: here a specialization constant and a constant defined in the
// same block. An `OpExecutionMode` or `OpExecutionModeId` that names its entry point
// (`$computeD`) is emitted as written, for that entry point only, even when other entry points
// call the helper. See https://github.com/shader-slang/slang/issues/13438.
//
// Helpers that hold only a `spirv_asm` block are inlined into their callers, so
// `computeA`..`computeD` see the block in their own body. `computeE`..`computeG` use helpers
// with other code too, which stay separate functions: there the mode goes to every entry point
// that reaches the helper, directly or through another helper.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_6
//TEST:SIMPLE(filecheck=COUNT): -target spirv -profile spirv_1_6
//TEST:SIMPLE(filecheck=SZ): -target spirv -profile spirv_1_6
//TEST:SIMPLE(filecheck=DP): -target spirv -profile spirv_1_6

// CHECK-DAG: OpCapability RayTracingOpacityMicromapExecutionModeKHR
// CHECK-DAG: OpExtension "SPV_KHR_opacity_micromap"
// CHECK-DAG: OpEntryPoint GLCompute %[[A:[A-Za-z0-9_]+]] "computeA"
// CHECK-DAG: OpEntryPoint GLCompute %[[B:[A-Za-z0-9_]+]] "computeB"
// CHECK-DAG: OpEntryPoint GLCompute %[[C:[A-Za-z0-9_]+]] "computeC"
// CHECK-DAG: OpEntryPoint GLCompute %[[D:[A-Za-z0-9_]+]] "computeD"
// CHECK-DAG: OpEntryPoint GLCompute %[[E:[A-Za-z0-9_]+]] "computeE"
// CHECK-DAG: OpEntryPoint GLCompute %[[F:[A-Za-z0-9_]+]] "computeF"
// CHECK-DAG: OpEntryPoint GLCompute %[[G:[A-Za-z0-9_]+]] "computeG"
// CHECK-DAG: OpExecutionModeId %[[A]] OpacityMicromapIdKHR %[[SPEC:[A-Za-z0-9_]+]]
// CHECK-DAG: OpExecutionModeId %[[B]] OpacityMicromapIdKHR %[[SPEC]]
// CHECK-DAG: OpExecutionModeId %[[C]] OpacityMicromapIdKHR %[[TRUE:[A-Za-z0-9_]+]]
// CHECK-DAG: OpExecutionModeId %[[D]] OpacityMicromapIdKHR %[[SPECD:[A-Za-z0-9_]+]]
// CHECK-DAG: OpExecutionModeId %[[E]] OpacityMicromapIdKHR %[[SPEC]]
// CHECK-DAG: OpExecutionModeId %[[F]] OpacityMicromapIdKHR %[[SPEC]]
// CHECK-DAG: OpExecutionModeId %[[G]] OpacityMicromapIdKHR %[[SPECD]]
// CHECK-DAG: OpFunctionCall %void %[[SHARED:[A-Za-z0-9_]+]]
// CHECK-DAG: %[[SHARED]] = OpFunction %void
// CHECK-DAG: OpExecutionMode %[[B]] SignedZeroInfNanPreserve 32
// CHECK-DAG: OpExecutionMode %[[D]] DenormPreserve 32
// CHECK-DAG: OpDecorate %[[SPEC]] SpecId 2
// CHECK-DAG: OpDecorate %[[SPECD]] SpecId 3
// CHECK-DAG: %[[SPEC]] = OpSpecConstantFalse %bool
// CHECK-DAG: %[[SPECD]] = OpSpecConstantFalse %bool
// CHECK-DAG: %[[TRUE]] = OpConstantTrue %bool

// One `OpExecutionModeId` per entry point: the `__entryPoint` line written twice in one block
// is emitted once per entry point, and the `$computeD` line is not applied to `computeA`, and is
// emitted once although the helper is inlined into two entry points. `computeC` calls a helper
// with an in-block constant twice: the two copies of the constant share one id, so it also gets
// the mode once. `computeE`, `computeF` and `computeG` get one each, and the `$computeG` line
// in a helper that only `computeE` calls is not applied to `computeE`.
// COUNT-COUNT-7: OpExecutionModeId
// COUNT-NOT: OpExecutionModeId

// The same for `OpExecutionMode`: the `__entryPoint` form reaches `computeB` only, which is the
// only caller, and the `$computeD` form is not applied to `computeA`.
// SZ-COUNT-1: SignedZeroInfNanPreserve 32
// SZ-NOT: SignedZeroInfNanPreserve 32
// DP-COUNT-1: DenormPreserve 32
// DP-NOT: DenormPreserve 32

[vk::constant_id(2)]
const bool kEnableOpacityMicromap = false;

[vk::constant_id(3)]
const bool kEnableOpacityMicromapOnD = false;

RWStructuredBuffer<uint> outputBuffer;

// Shared helper: the execution mode lands on whichever entry points call it.
void enableOpacityMicromapFromSpecConstant()
{
    spirv_asm
    {
        OpCapability RayTracingOpacityMicromapExecutionModeKHR;
        OpExtension "SPV_KHR_opacity_micromap";
        OpExecutionModeId __entryPoint OpacityMicromapIdKHR $kEnableOpacityMicromap;
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

void preserveSignedZeroInfNan()
{
    spirv_asm
    {
        OpCapability SignedZeroInfNanPreserve;
        OpExecutionMode __entryPoint SignedZeroInfNanPreserve 32
    };
}

// Names `computeD` explicitly, so the modes apply to `computeD` only, although `computeA`
// calls this helper too.
void setModesOnComputeD()
{
    spirv_asm
    {
        OpCapability RayTracingOpacityMicromapExecutionModeKHR;
        OpExtension "SPV_KHR_opacity_micromap";
        OpCapability DenormPreserve;
        OpExecutionModeId $computeD OpacityMicromapIdKHR $kEnableOpacityMicromapOnD;
        OpExecutionMode $computeD DenormPreserve 32
    };
}

// Not inlined (it also stores), so it stays one function that several entry points call.
void enableFromSharedHelper(uint i)
{
    outputBuffer[i] = 5;
    spirv_asm
    {
        OpCapability RayTracingOpacityMicromapExecutionModeKHR;
        OpExtension "SPV_KHR_opacity_micromap";
        OpExecutionModeId __entryPoint OpacityMicromapIdKHR $kEnableOpacityMicromap
    };
}

// Reaches `enableFromSharedHelper` through another call.
void callSharedHelper(uint i)
{
    outputBuffer[i + 1] = 6;
    enableFromSharedHelper(i);
}

// Not inlined, and names `computeG`, although only `computeE` calls it.
void setModeOnComputeGFromSharedHelper(uint i)
{
    outputBuffer[i] = 7;
    spirv_asm
    {
        OpCapability RayTracingOpacityMicromapExecutionModeKHR;
        OpExtension "SPV_KHR_opacity_micromap";
        OpExecutionModeId $computeG OpacityMicromapIdKHR $kEnableOpacityMicromapOnD
    };
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeA(uint3 tid: SV_DispatchThreadID)
{
    enableOpacityMicromapFromSpecConstant();
    enableOpacityMicromapFromSpecConstant();
    setModesOnComputeD();
    outputBuffer[tid.x] = 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeB(uint3 tid: SV_DispatchThreadID)
{
    enableOpacityMicromapFromSpecConstant();
    preserveSignedZeroInfNan();
    outputBuffer[tid.x] = 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeC(uint3 tid: SV_DispatchThreadID)
{
    enableOpacityMicromap();
    enableOpacityMicromap();
    outputBuffer[tid.x] = 3;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeD(uint3 tid: SV_DispatchThreadID)
{
    setModesOnComputeD();
    outputBuffer[tid.x] = 4;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeE(uint3 tid: SV_DispatchThreadID)
{
    enableFromSharedHelper(tid.x);
    setModeOnComputeGFromSharedHelper(tid.x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeF(uint3 tid: SV_DispatchThreadID)
{
    callSharedHelper(tid.x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeG(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = 8;
}
