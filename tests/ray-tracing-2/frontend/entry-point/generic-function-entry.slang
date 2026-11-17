//TEST:SIMPLE: -experimental-feature -entry genericMain -stage compute -no-codegen
//TEST:SIMPLE: -experimental-feature -entry genericMain -specialize Q -stage compute -no-codegen
//TEST:SIMPLE: -entry genericMain -specialize Q -stage compute -no-codegen

// Enabling the structural ray-tracing feature routes every entry-point lookup through the
// structural selector before ordinary function lookup. A bare generic *function* name must
// still resolve through the ordinary path there: the structural selector enters the generic's
// inner declaration, sees it is not a stage struct, and declines without diagnosing.

struct Q {}

[shader("compute")]
[numthreads(1, 1, 1)]
void genericMain<T>() {}
