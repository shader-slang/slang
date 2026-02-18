// output-no-entry-point.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-o something.dxbc -entry vsMain -stage vertex -entry fsMain -stage fragment
//CHECK: the output path 'something.dxbc' is not associated with any entry point
