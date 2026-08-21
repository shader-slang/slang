// enum-case-attribute.slang

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -shaderobj

[__AttributeUsage(_AttributeTargets.EnumCase)]
struct DisplayNameAttribute
{
    string name;
}

enum LogLevel
{
    [DisplayName("Detailed diagnostics")]
    Verbose,

    Info,

    [DisplayName("Errors only")]
    Error = 10,
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    LogLevel level = LogLevel.Error;
    // CHECK: 10
    outputBuffer[0] = (int)level;
}
