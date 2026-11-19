// Checks argument-count and type errors of the [RootSignature] attribute,
// which accepts exactly one string argument on functions.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl

[RootSignature()]
/*CHECK:
 ^^^^^^^^^^^^^ wrong number of attribute arguments
 ^^^^^^^^^^^^^ attribute 'RootSignature' expects 1 arguments (0 provided)
*/
[RootSignature("a", "b")]
/*CHECK:
 ^^^^^^^^^^^^^ too many arguments to call
 ^^^^^^^^^^^^^ too many arguments to call (got 2, expected 1)
*/
[RootSignature(1)]
//CHECK:       ^ type mismatch in expression
//CHECK:       ^ expected an expression of type 'String', got 'int'
//CHECK:       ^ explicit conversion from 'int' to 'String' is possible
[numthreads(1, 1, 1)]
void main() {}
