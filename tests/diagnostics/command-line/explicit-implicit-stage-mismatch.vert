// explicit-implicit-stage-mismatch.vert

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage fragment
//CHECK: the stage specified for entry point 'main' ('pixel') does not match the stage implied by the source file name ('vertex')

void main() {}
