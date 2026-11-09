//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hgbc -entry main

[host("graph")]
//CHECK: attribute 'host' expects 0 arguments (1 provided)
void main() {}
