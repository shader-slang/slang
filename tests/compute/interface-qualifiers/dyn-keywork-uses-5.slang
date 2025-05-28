//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK_2025_OR_EXP): -target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_2025_OR_EXP): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -enable-experimental-dynamic-dispatch

// Validate AST side of `dyn` keyword use-cases.
// dyn interface cannot return `some` type or have `some` param.
interface IFoo
{
}
dyn interface ReturnsSomeTypeAndSomeParam
{
// CHECK_2026-DAG: error 33086
// CHECK_2026-DAG: error 33087

// CHECK_2025_OR_EXP-NOT: error 33086
// CHECK_2025_OR_EXP-NOT: error 33087

    some IFoo getIFoo()
	{
		return IFooStruct(1);
	}

    void lookAtIFoo(some IFoo v)
	{
	}
}

RWStructuredBuffer<int> outputBuffer;
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	outputBuffer[0] = 0;
}
