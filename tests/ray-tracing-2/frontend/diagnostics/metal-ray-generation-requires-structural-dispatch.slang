//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal

// Importing the structural module tentatively admits Metal's logical ray-generation stage so
// generic witness specialization can expose a hidden structural trace. When specialization finds
// no trace or callable dispatch, Metal lowering must issue a controlled diagnostic instead of
// leaving the unsupported logical stage for the emitter.

import slang.raytracing;

[shader("raygeneration")]
void main()
/*CHECK:
     ^^^^ Metal ray-generation entry point requires structural dispatch
     ^^^^ this Metal ray-generation entry point does not reach 'RayTracer.trace' or 'RayTracer.callShader'
*/
{}
