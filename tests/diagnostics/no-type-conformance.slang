//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute -o no-type-conformance.hlsl

//CHECK: error 50100: No type conformances are found for interface 'IFoo'
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