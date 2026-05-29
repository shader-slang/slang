//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// associatedtype is not allowed at module scope (caught by parser).

associatedtype BadAssoc;
//CHECK:       ^^^^^^^^ declaration not allowed here
//CHECK:       ^^^^^^^^ associatedtype is not allowed here.
