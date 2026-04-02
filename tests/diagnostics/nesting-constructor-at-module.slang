//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Constructors are NOT allowed at module scope.

__init() {}
//CHECK: declaration not allowed here
//CHECK: __init is not allowed here.
