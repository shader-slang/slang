//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -conformance "Impl:NonExistentInterface=0"

interface IFoo { int get(); }
struct Impl : IFoo { int get() { return 1; } }

RWStructuredBuffer<int> out;

[numthreads(1,1,1)]
void computeMain() { out[0] = 0; }

//CHECK: invalid conformance option 'Impl:NonExistentInterface=0', type 'NonExistentInterface' is not found.
//CHECK-NOT: undefined identifier
//CHECK-NOT: cannot create type conformance
