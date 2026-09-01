//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
//
// Negative regression test for https://github.com/shader-slang/slang/issues/12326

enum ErrorCodes : uint
{
    Success = 0,
    InvalidNegativeValue = 1,
}

// require that input is positive
uint validateInput(int n) throws ErrorCodes
{
    // throw statement used to not require the terminating semicolon,
    // accepting erroneous input

    if (n < 0)
        throw ErrorCodes.InvalidNegativeValue

    return (uint)n;
/*CHECK:
    ^^^^^^ unexpected token
    ^^^^^^ unexpected identifier, expected ';'
*/
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
