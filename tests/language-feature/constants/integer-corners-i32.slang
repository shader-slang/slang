//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly
//TEST_INPUT:ubuffer(data=[7 7 7 7 7 7 7 7], stride=4, count=8):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

static const int32_t i32zero  = 0;
static const int32_t i32one   = 1;
static const int32_t i32min = int32_t(-0x80000000); // -2147483648
static const int32_t i32max = int32_t(0x7fffffff);  // 2147483647

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // BUF: type: int32_t
    outputBuffer[0] = i32max + i32one; // BUF-NEXT: -2147483648
    outputBuffer[1] = i32min - i32one; // BUF-NEXT: 2147483647
    outputBuffer[2] = i32max * i32max; // BUF-NEXT: 1
    outputBuffer[3] = i32max % i32max; // BUF-NEXT: 0
    outputBuffer[4] = i32one % i32max; // BUF-NEXT: 1
    outputBuffer[5] = i32max % i32one; // BUF-NEXT: 0
    outputBuffer[6] = i32max / i32max; // BUF-NEXT: 1
    outputBuffer[7] = i32one / i32max; // BUF-NEXT: 0
}
