// Test that [numthreads(-1,1,1)] produces error E31102.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain

[numthreads(-1,1,1)]
/*CHECK:
 ^^^^^^^^^^ invalid 'numthreads' value
 ^^^^^^^^^^ expected a positive integer in 'numthreads' attribute, got '-1'
*/
void computeMain() {}
