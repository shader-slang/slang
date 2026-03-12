//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain -emit-spirv-directly
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain( uint2 dispatchThreadID : SV_DispatchThreadID )
{
    float4 vecVal = float4(0);
    outputBuffer[0] = vecVal.xxtyxx;
//CHECK:                     ^^^^^^ member not found
//CHECK:                     ^^^^^^ 'xxtyxx' is not a member of 'vector<float,4>'.
}
