//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute -o no-type-conformance.hlsl

interface IFoo
{
	float get();
}

void foo()
{
	IFoo obj;
	obj.get();
//CHECK:^ cannot dispatch on uninitialized interface
//CHECK:^ Cannot dynamically dispatch on potentially uninitialized interface object 'IFoo'.
//CHECK:^ cannot dispatch on uninitialized interface
//CHECK:^ Cannot dynamically dispatch on potentially uninitialized interface object 'IFoo'.
}

[numthreads(1, 1, 1)]
void computeMain(uint3       dispatchThreadID : SV_DispatchThreadID)
{
    foo();
}
