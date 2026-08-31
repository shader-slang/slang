// A stage-bearing extension belongs to the language convention that defines it. When `-lang hlsl`
// overrides the GLSL meaning of `.vert`, the extension must not continue supplying a vertex stage.
// Keep system-value semantics in the signature so this also verifies that a missing stage stops
// validation before any stage-specific semantic capability lookup.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/explicit-language-drops-extension-stage.vert -entry main -no-codegen
//CHECK: explicitly requested source language overrides the language implied by the extension of input file 'tests/diagnostics/explicit-language-drops-extension-stage.vert'

float4 main(float4 position : POSITION) : SV_Position
/*CHECK:
       ^^^^ no stage specified for entry point
       ^^^^ no stage specified for entry point 'main'; use either a '[shader("name")]' function attribute or the '-stage <name>' command-line option to specify a stage
*/
{
    return position;
}
