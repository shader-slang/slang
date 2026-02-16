//DIAGNOSTIC_TEST:SIMPLE(diag=diag,non-exhaustive): -target spirv

// Test that we can diagnose an error when there is a circular include.
// The diagnostic happens in b.slang when it tries to include a.slang again

#include "b.slang"
