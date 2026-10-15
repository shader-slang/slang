// A `__stage_switch` in a function that no entry point reaches takes its
// `default:` case. A `[CudaDeviceExport]` function that the kernel does not call
// has no stage to specialize to.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

RWStructuredBuffer<uint> outputBuffer;
groupshared uint sharedVal;

[noinline]
uint stageValue()
{
    __stage_switch
    {
    case compute:
        return 1;
    default:
        return 2;
    }
}

// `workgroupUniformLoad` synchronizes only in workgroup stages, so the
// unreachable export gets its plain-load `default:` case.
[CudaDeviceExport]
uint readShared()
{
    return workgroupUniformLoad(sharedVal);
}

[CudaDeviceExport]
uint exportedStageValue()
{
    return stageValue();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 7;
}

// CHECK: uint readShared()
// CHECK-NOT: __syncthreads
// CHECK: return *&sharedVal
// CHECK: uint stageValue
// CHECK-NEXT: {
// CHECK-NEXT: return 2U;
// CHECK: uint exportedStageValue()
// CHECK-NEXT: {
// CHECK-NEXT: return stageValue
