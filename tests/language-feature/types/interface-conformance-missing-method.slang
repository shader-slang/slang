// interface-conformance-missing-method.slang

// Test that the compiler emits an error when a struct declares conformance
// but does not implement a required method.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

interface IRequired
{
    int compute(int x);
//CHECK:^^^^^^^ see declaration of 'compute'
}

struct Incomplete : IRequired
//CHECK:            ^^^^^^^^^ missing interface member
//CHECK:            ^^^^^^^^^ type 'Incomplete' does not provide required interface member 'compute'
{
    int data;
}
