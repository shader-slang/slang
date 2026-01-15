//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly
//TEST_INPUT:ubuffer(data=[7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7], stride=8, count=8):out,name=outputBuffer
RWStructuredBuffer<int64_t> outputBuffer;

static const int64_t i64zero  = 0;
static const int64_t i64one   = 1;
static const int64_t i64min = int64_t(-0x8000000000000000LL); // -9223372036854775808
static const int64_t i64max = int64_t(0x7fffffffffffffffLL);  // 9223372036854775807

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // BUF: type: int64_t
    outputBuffer[0] = i64max + i64one; // BUF-NEXT: -9223372036854775808
    outputBuffer[1] = i64min - i64one; // BUF-NEXT: 9223372036854775807
    outputBuffer[2] = i64max * i64max; // BUF-NEXT: 1
    outputBuffer[3] = i64max % i64max; // BUF-NEXT: 0
    outputBuffer[4] = i64one % i64max; // BUF-NEXT: 1
    outputBuffer[5] = i64max % i64one; // BUF-NEXT: 0
    outputBuffer[6] = i64max / i64max; // BUF-NEXT: 1
    outputBuffer[7] = i64one / i64max; // BUF-NEXT: 0
}

