//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly
//TEST_INPUT:ubuffer(data=[7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7], stride=2, count=8):out,name=outputBuffer
RWStructuredBuffer<uint16_t> outputBuffer;

static const uint16_t u16zero  = 0;
static const uint16_t u16one   = 1;
static const uint16_t u16min = uint16_t(0);      // 0
static const uint16_t u16max = uint16_t(0xffff); // 65535

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // BUF: type: uint16_t
    outputBuffer[0] = u16max + u16one; // BUF-NEXT: 0
    outputBuffer[1] = u16min - u16one; // BUF-NEXT: 65535
    outputBuffer[2] = u16max * u16max; // BUF-NEXT: 1
    outputBuffer[3] = u16max % u16max; // BUF-NEXT: 0
    outputBuffer[4] = u16one % u16max; // BUF-NEXT: 1
    outputBuffer[5] = u16max % u16one; // BUF-NEXT: 0
    outputBuffer[6] = u16max / u16max; // BUF-NEXT: 1
    outputBuffer[7] = u16one / u16max; // BUF-NEXT: 0
}

