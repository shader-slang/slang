//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -conformance "NonExistent:IFoo=0"

interface IFoo { int get(); }
struct Impl : IFoo { int get() { return 1; } }

RWStructuredBuffer<int> out;

[numthreads(1,1,1)]
void computeMain() { out[0] = 0; }

//CHECK: invalid conformance option 'NonExistent:IFoo=0', type 'NonExistent' is not found.
