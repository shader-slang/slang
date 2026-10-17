//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Each declaration in this circular initializer pair should produce one error.
// Folding the initializers must not duplicate the circularity diagnostics.
export static const int A = B;
//CHECK:                ^ the initial-value expression for variable 'A' depends on the value of the variable itself
static const int B = A;
//CHECK:         ^ the initial-value expression for variable 'B' depends on the value of the variable itself
