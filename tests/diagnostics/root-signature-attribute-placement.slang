// Checks the placement restriction of the [RootSignature] attribute:
// it is only valid on functions, not on other declarations.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl

[RootSignature("x")]
/*CHECK:
 ^^^^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^^^^ attribute 'RootSignature' is not valid here
*/
struct S {}
[numthreads(1, 1, 1)]
void main() {}
