//DIAGNOSTIC_TEST:SIMPLE(diag=DISABLED): -entry Plain -stage miss -no-codegen
// The undefined-name diagnostic comes from the command line and has no source location.
//DIAGNOSTIC_TEST:SIMPLE(diag=MISSING): -experimental-feature -entry missing -stage miss -no-codegen
//MISSING: undefined identifier
//MISSING: undefined identifier 'missing'.
//TEST:SIMPLE: -entry computeMain -stage compute -no-codegen
//TEST:SIMPLE: -experimental-feature -entry computeMain -stage compute -no-codegen

// Selecting a struct without the experimental flag must use ordinary function lookup diagnostics.
// A missing name must retain that diagnostic even when structural lookup is enabled.

// Missing-entry errors use the module location, which is this first declaration.
struct Plain {}
/*DISABLED:
^^^^^^ no function found matching entry point name 'Plain'
*/
/*MISSING:
^^^^^^ no function found matching entry point name 'missing'
*/

[numthreads(1, 1, 1)]
void computeMain() {}
