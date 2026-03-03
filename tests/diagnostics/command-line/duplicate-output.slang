// duplicate-output.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-entry main -stage compute -o myKernel.dxbc -o myKernel.dxbc
//CHECK: multiple output paths have been specified entry point 'main' on target 'dxbc'
