// Tests argument validation of the [RootSignature] attribute:
// it accepts exactly one argument of type string.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl

[RootSignature()]
/*CHECK:
 ^^^^^^^^^^^^^ wrong number of attribute arguments
 ^^^^^^^^^^^^^ attribute 'RootSignature' expects 1 arguments (0 provided)
*/
[RootSignature(1)]
//CHECK:       ^ type mismatch in expression
//CHECK:       ^ expected an expression of type 'String', got 'int'
//CHECK:       ^ explicit conversion from 'int' to 'String' is possible
[numthreads(1, 1, 1)]
void main() {}
