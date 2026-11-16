//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry NotAStage -stage closesthit -no-codegen

import slang.raytracing.portable;

struct NotAStage
/*CHECK:
       ^^^^^^^^^ invalid structural ray-tracing entry point
       ^^^^^^^^^ struct 'NotAStage' does not implement an executable structural ray-tracing stage interface
*/
{}
