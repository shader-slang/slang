//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -emit-spirv-directly
// CHECK: OpEntryPoint
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain( uint2 dispatchThreadID : SV_DispatchThreadID )
{
    float4 vecVal = float4(0);
    outputBuffer[0] = vecVal.xxxxx;
}
