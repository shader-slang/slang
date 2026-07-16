//TEST:SIMPLE(filecheck=VALID): -stage compute -entry computeMain -target spirv -DTHREAD_COUNT_DEFAULT=8
//TEST:SIMPLE(filecheck=INVALID): -stage compute -entry computeMain -target spirv -DTHREAD_COUNT_DEFAULT=3

// VALID-DAG: OpExecutionModeId %computeMain LocalSizeId
// VALID-DAG: OpExecutionMode %computeMain DerivativeGroupQuads{{NV|KHR}}
// INVALID: error[E31210]

[vk::constant_id(0)]
const int threadCountX = THREAD_COUNT_DEFAULT;

RWStructuredBuffer<float> outputBuffer;

[DerivativeGroupQuad]
[numthreads(threadCountX, 8, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[0] = ddx_fine(float(dispatchThreadID.x));
}
