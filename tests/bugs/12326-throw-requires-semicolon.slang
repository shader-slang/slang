//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -warnings-as-errors all

// Positive regression test for https://github.com/shader-slang/slang/issues/12326

enum ErrorCodes : uint
{
    Success = 0,
    InvalidNegativeValue = 1,
}

// require that input is positive
uint validateInput(int n) throws ErrorCodes
{
    // throw statement used to not require the terminating semicolon,
    // causing compilation issues in certain cases. For example,
    // the below 'if' would fail to compile.

    if (n < 0)
        throw ErrorCodes.InvalidNegativeValue; // note here
    else
        return (uint)n;
}

struct Result
{
    uint value;
    uint errorCode; // 0 for success
}

StructuredBuffer<int> inputBuffer;
RWStructuredBuffer<Result> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Result res = { };

    do
    {
        res.value = try validateInput(inputBuffer[0]);
    }
    catch (ErrorCodes err)
    {
        res.errorCode = err;
    }

    outputBuffer[0] = res;
}

//CHECK: %computeMain = OpFunction
