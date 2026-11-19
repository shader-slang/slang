// Tests argument validation of the [RootSignature] attribute:
// it takes exactly one argument of type string.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl

[RootSignature()]
//CHECK: error: wrong number of attribute arguments
//CHECK: attribute 'RootSignature' expects 1 arguments (0 provided)
[RootSignature(1)]
//CHECK: error: type mismatch in expression
//CHECK: expected an expression of type 'String', got 'int'
[numthreads(1, 1, 1)]
void main() {}
