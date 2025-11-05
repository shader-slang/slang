//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute -o no-type-conformance.hlsl

//CHECK: error 50101: Cannot dynamically dispatch on potentially uninitialized interface object 'IFoo'
interface IFoo
{
	float get();
}

void foo()
{
	IFoo obj;
	obj.get();
}

[numthreads(1, 1, 1)]
void computeMain(uint3       dispatchThreadID : SV_DispatchThreadID)
{
    foo();
}