//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main

//CHECK: E38007
void main() {}
/*CHECK:
     ^^^^ no stage specified for entry point
*/
