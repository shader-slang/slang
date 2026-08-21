// enum-case-attribute.slang

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -shaderobj

[__AttributeUsage(_AttributeTargets.EnumCase)]
struct DisplayNameAttribute
{
    string name;
}

[__AttributeUsage(_AttributeTargets.EnumCase)]
struct PriorityAttribute
{
    int level;
}

enum LogLevel
{
    // Multiple attributes on a single case.
    [DisplayName("Detailed diagnostics")]
    [Priority(1)]
    Verbose,

    Info,

    // Attribute on the last case, which has no trailing comma.
    [DisplayName("Errors only")]
    Error = 10
}

// A case name that is also a bareword modifier keyword must still parse (the
// attribute parser must not consume it as a modifier).
enum FilterMode
{
    point,
    linear,

    [DisplayName("Anisotropic filtering")]
    anisotropic
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    LogLevel level = LogLevel.Error;
    FilterMode mode = FilterMode.linear;
    // CHECK: 11
    outputBuffer[0] = (int)level + (int)mode;
}
