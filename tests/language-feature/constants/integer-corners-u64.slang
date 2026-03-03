//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly
//TEST_INPUT:ubuffer(data=[7 7 7 7 7 7 7 7], stride=8, count=8):out,name=outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

static const uint64_t u64zero  = 0;
static const uint64_t u64one   = 1;
static const uint64_t u64min = uint64_t(0);                    // 0
static const uint64_t u64max = uint64_t(0xffffffffffffffffULL); // 18446744073709551615

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // BUF: type: uint64_t
    outputBuffer[0] = u64max + u64one; // BUF-NEXT: 0
    outputBuffer[1] = u64min - u64one; // BUF-NEXT: 18446744073709551615
    outputBuffer[2] = u64max * u64max; // BUF-NEXT: 1
    outputBuffer[3] = u64max % u64max; // BUF-NEXT: 0
    outputBuffer[4] = u64one % u64max; // BUF-NEXT: 1
    outputBuffer[5] = u64max % u64one; // BUF-NEXT: 0
    outputBuffer[6] = u64max / u64max; // BUF-NEXT: 1
    outputBuffer[7] = u64one / u64max; // BUF-NEXT: 0
}


