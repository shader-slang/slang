//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly
//TEST_INPUT:ubuffer(data=[7 7 7 7 7 7 7 7], stride=1, count=8):out,name=outputBuffer
RWStructuredBuffer<int8_t> outputBuffer;

static const int8_t i8zero  = 0;
static const int8_t i8one   = 1;
static const int8_t i8min = int8_t(-0x80); // -128
static const int8_t i8max = int8_t(0x7f);  // 127

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // BUF: type: int8_t
    outputBuffer[0] = i8max + i8one; // BUF-NEXT: -128
    outputBuffer[1] = i8min - i8one; // BUF-NEXT: 127
    outputBuffer[2] = i8max * i8max; // BUF-NEXT: 1
    outputBuffer[3] = i8max % i8max; // BUF-NEXT: 0
    outputBuffer[4] = i8one % i8max; // BUF-NEXT: 1
    outputBuffer[5] = i8max % i8one; // BUF-NEXT: 0
    outputBuffer[6] = i8max / i8max; // BUF-NEXT: 1
    outputBuffer[7] = i8one / i8max; // BUF-NEXT: 0
}

