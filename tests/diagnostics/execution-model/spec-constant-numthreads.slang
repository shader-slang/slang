// Test that specialization constants in numthreads produce an error on non-SPIRV targets.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-target hlsl -entry main

[[vk::constant_id(0)]] const int X = 4;
[numthreads(X, 1, 1)]
/*CHECK:
 ^^^^^^^^^^ Specialization constants are not supported in the 'numthreads' attribute
*/
[shader("compute")]
void main() {}
