//TEST:COMPARE_COMPUTE(targets=all-dx11-dx12-wgpu, filecheck-buffer=BUF):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[200], stride=4):name input1
RWStructuredBuffer<uint8_t> input1;

//TEST_INPUT:ubuffer(data=[35000], stride=4):name input2
RWStructuredBuffer<uint16_t> input2;

//TEST_INPUT:ubuffer(data=[201], stride=4):name input3
RWStructuredBuffer<uint8_t> input3;

//
// Tests unsigned to signed casts to wider int.
//

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;

    // BUF: 200
    outputBuffer[index] = input1[0];

    // BUF: 35000
    outputBuffer[index + 1] = input2[0];

    // BUF: 201
    int16_t a = input3[0];
    outputBuffer[index + 2] = a;
}
