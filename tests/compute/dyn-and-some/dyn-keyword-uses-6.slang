//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -stage compute -entry computeMain -lang slang -std 2026
// Validate AST side of `dyn` keyword use-cases.

// CHECK_2026: error 20001: unexpected identifier
dyn struct ReturnsSomeTypeAndSomeParam
{
}

RWStructuredBuffer<int> outputBuffer;
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	outputBuffer[0] = 0;
}
