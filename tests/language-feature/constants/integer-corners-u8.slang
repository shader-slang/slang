//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly
//TEST_INPUT:ubuffer(data=[7 7 7 7 7 7 7 7], stride=1, count=8):out,name=outputBuffer
RWStructuredBuffer<uint8_t> outputBuffer;

static const uint8_t u8zero  = 0;
static const uint8_t u8one   = 1;
static const uint8_t u8min = uint8_t(0);    // 0
static const uint8_t u8max = uint8_t(0xff); // 255

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // BUF: type: uint8_t
    outputBuffer[0] = u8max + u8one; // BUF-NEXT: 0
    outputBuffer[1] = u8min - u8one; // BUF-NEXT: 255
    outputBuffer[2] = u8max * u8max; // BUF-NEXT: 1
    outputBuffer[3] = u8max % u8max; // BUF-NEXT: 0
    outputBuffer[4] = u8one % u8max; // BUF-NEXT: 1
    outputBuffer[5] = u8max % u8one; // BUF-NEXT: 0
    outputBuffer[6] = u8max / u8max; // BUF-NEXT: 1
    outputBuffer[7] = u8one / u8max; // BUF-NEXT: 0
}

