//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// missing entrypoint attribute
void vertMain()
{}

//CHECK: SPIR-V output contains no exported symbols
//CHECK: output SPIR-V contains no exported symbols. Please make sure to specify at least one entrypoint.
