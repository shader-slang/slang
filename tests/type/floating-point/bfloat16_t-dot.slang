//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry main
//SPIRV: OpCapability BFloat16DotProductKHR

StructuredBuffer<vector<bfloat16_t, 3>> inputBuffer;
RWStructuredBuffer<bfloat16_t> outputBuffer;

[numthreads(1, 1, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = dispatchThreadID.x;
    vector<bfloat16_t, 3> vec0 = inputBuffer[idx * 2 + 0];
    vector<bfloat16_t, 3> vec1 = inputBuffer[idx * 2 + 1];
    //SPIRV: OpDot %bfloat16
    outputBuffer[idx * 2 + 0] = dot(vec0, vec1);
}
