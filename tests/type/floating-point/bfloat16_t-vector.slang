//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry main

StructuredBuffer<bfloat16_t> inputBuffer;
RWStructuredBuffer<vector<bfloat16_t, 3>> outputBuffer;

[numthreads(1, 1, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = dispatchThreadID.x;
    bfloat16_t val0 = inputBuffer[idx * 3 + 0];
    bfloat16_t val1 = inputBuffer[idx * 3 + 1];
    bfloat16_t val2 = inputBuffer[idx * 3 + 2];
    //SPIRV: [[BF16DATA:%.*]] = OpCompositeConstruct %v3bfloat16
    let vec = vector<bfloat16_t, 3>(val0, val1, val2);
    //SPIRV: [[OUT:%.*]] = OpAccessChain %_ptr_StorageBuffer_v3bfloat16 %outputBuffer %int_0 %idx
    //SPIRV: OpStore [[OUT]] [[BF16DATA]]
    outputBuffer[idx] = vec;
}
