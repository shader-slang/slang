//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// Error-suppression test for gh-8550: when the initializer already has a prior error
// (undefined identifier), E31226 must NOT be emitted as a cascading diagnostic.
// The errorCountBeforeInitCheck mechanism implements this suppression.

static const int bad = undefinedSymbol * 2;
//CHECK:               ^^^^^^^^^^^^^^^ undefined identifier
//CHECK-NOT: 31226
