//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly
//TEST_INPUT:ubuffer(data=[0xAAAAAAAA 0xAAAAAAAA], stride=2, count=8):out,name=outputBuffer
RWStructuredBuffer<int16_t> outputBuffer;

static const int16_t i16zero  = 0;
static const int16_t i16one   = 1;
static const int16_t i16min = int16_t(-0x8000); // -32768
static const int16_t i16max = int16_t(0x7fff);  // 32767

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // BUF: type: int16_t
    outputBuffer[0] = i16max + i16one; // BUF-NEXT: -32768
    outputBuffer[1] = i16min - i16one; // BUF-NEXT: 32767
    outputBuffer[2] = i16max * i16max; // BUF-NEXT: 1
    outputBuffer[3] = i16max % i16max; // BUF-NEXT: 0
    outputBuffer[4] = i16one % i16max; // BUF-NEXT: 1
    outputBuffer[5] = i16max % i16one; // BUF-NEXT: 0
    outputBuffer[6] = i16max / i16max; // BUF-NEXT: 1
    outputBuffer[7] = i16one / i16max; // BUF-NEXT: 0
}

