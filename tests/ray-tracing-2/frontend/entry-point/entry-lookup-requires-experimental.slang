//DIAGNOSTIC_TEST:SIMPLE(filecheck=DISABLED): -entry Plain -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=MISSING): -experimental-feature -entry missing -stage miss -no-codegen
//TEST:SIMPLE: -entry computeMain -stage compute -no-codegen
//TEST:SIMPLE: -experimental-feature -entry computeMain -stage compute -no-codegen

// Selecting a struct without the experimental flag must use ordinary function lookup diagnostics.
// A missing name must retain that diagnostic even when structural lookup is enabled.
//DISABLED-NOT: structural ray-tracing
//DISABLED: no function found matching entry point name 'Plain'
//DISABLED-NOT: structural ray-tracing
//MISSING-NOT: structural ray-tracing
//MISSING: error[E30015]: undefined identifier
//MISSING-NEXT: error[E38000]: no function found matching entry point name 'missing'
//MISSING-NOT: structural ray-tracing

struct Plain {}

[numthreads(1, 1, 1)]
void computeMain() {}
