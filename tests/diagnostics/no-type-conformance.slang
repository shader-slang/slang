//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute -o no-type-conformance.hlsl

interface IFoo
{
	float get();
}

void foo()
{
	IFoo obj = createDynamicObject<IFoo>(0, 0);
//CHECK:                             ^ no type conformances found
//CHECK:                             ^ No type conformances are found for interface 'IFoo'. Code generation for current target requires at least one implementation type present in the linkage.
	obj.get();
}

[numthreads(1, 1, 1)]
void computeMain(uint3       dispatchThreadID : SV_DispatchThreadID)
{
    foo();
}