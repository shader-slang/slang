// Test that [WaveSize(3)] produces error E31103 (must be power of 2 between 4-128).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain

[WaveSize(3)]
/*CHECK:
 ^^^^^^^^ invalid 'WaveSize' value
 ^^^^^^^^ expected a power of 2 between 4 and 128, inclusive, in 'WaveSize' attribute, got '3'
*/
[numthreads(1,1,1)]
void computeMain() {}
