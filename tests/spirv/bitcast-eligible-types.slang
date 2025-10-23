// bitcast-eligible-types.slang
// Test that eligible types (basic types, vectors of basic types) use the optimization

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// Eligible types should use direct OpBitcast on the whole type (no lowering/splitting)
// CHECK-DAG: OpBitcast %float %
// CHECK-DAG: OpBitcast %v2float %
// CHECK-DAG: OpBitcast %v3float %
// CHECK-DAG: OpBitcast %v4uint %
// CHECK-DAG: OpBitcast %uint %

//TEST_INPUT:ubuffer(data=[1078523331 1073741824 1077936128 1082130432 286331153 573662306 1065353216 1073741824 1077936128 1084227584], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Use input data to prevent constant folding
    
    // Basic types
    uint u1 = inputBuffer[0];
    float f1 = bit_cast<float>(u1);
    
    int i1 = asint(inputBuffer[1]);
    uint u1_result = bit_cast<uint>(i1);
    
    // Vector types - direct vector-to-vector bitcasts
    uint2 u2 = uint2(inputBuffer[2], inputBuffer[3]);
    float2 f2 = bit_cast<float2>(u2);
    
    // Create int3 directly, then bitcast to float3
    int3 i3_input = int3(asint(inputBuffer[4]), asint(inputBuffer[5]), asint(inputBuffer[6]));
    float3 f3 = bit_cast<float3>(i3_input);
    
    // Create float4 directly, then bitcast to uint4
    float4 f4_input = float4(asfloat(inputBuffer[7]), asfloat(inputBuffer[8]), asfloat(inputBuffer[9]), asfloat(inputBuffer[0]));
    uint4 u4 = bit_cast<uint4>(f4_input);
    
    // Store results to prevent optimization
    outputBuffer[0] = asuint(f1);
    outputBuffer[1] = u1_result;
    outputBuffer[2] = asuint(f2.x);
    outputBuffer[3] = asuint(f3.x);
    outputBuffer[4] = u4.x;
}
