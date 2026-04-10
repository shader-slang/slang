//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly
//TEST_INPUT:ubuffer(data=[7 7 7 7 7 7 7 7], stride=4, count=8):out,name=outputBuffer
RWStructuredBuffer<uint32_t> outputBuffer;

static const uint32_t u32zero  = 0;
static const uint32_t u32one   = 1;
static const uint32_t u32min = uint32_t(0);          // 0
static const uint32_t u32max = uint32_t(0xffffffff); // 4294967295

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // BUF: type: uint32_t
    outputBuffer[0] = u32max + u32one; // BUF-NEXT: 0
    outputBuffer[1] = u32min - u32one; // BUF-NEXT: 4294967295
    outputBuffer[2] = u32max * u32max; // BUF-NEXT: 1
    outputBuffer[3] = u32max % u32max; // BUF-NEXT: 0
    outputBuffer[4] = u32one % u32max; // BUF-NEXT: 1
    outputBuffer[5] = u32max % u32one; // BUF-NEXT: 0
    outputBuffer[6] = u32max / u32max; // BUF-NEXT: 1
    outputBuffer[7] = u32one / u32max; // BUF-NEXT: 0
}


