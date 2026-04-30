// Test that zero values in [numthreads] produce errors.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain

[numthreads(0, 1, 1)]
/*CHECK:
 ^^^^^^^^^^ invalid 'numthreads' value
 ^^^^^^^^^^ expected a positive integer in 'numthreads' attribute, got '0'
*/
void computeMain()
{}
