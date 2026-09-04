// A stage-bearing extension belongs to the language convention that defines it. When `-lang hlsl`
// overrides the GLSL meaning of `.vert`, the extension must not continue supplying a vertex stage.
// Keep both a valid and an unknown system-value semantic in the signature so this exhaustive
// diagnostic test also verifies that the missing-stage error stops before accessor resolution
// calls `getAtomFromStage()` with `Stage::Unknown`, which is outside that helper's contract. The
// otherwise independent `SV_Foo` error is intentionally deferred until a stage is available.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/explicit-language-drops-extension-stage.vert -entry main -no-codegen
//CHECK: explicitly requested source language overrides the language implied by the extension of input file 'tests/diagnostics/explicit-language-drops-extension-stage.vert'

float4 main(float4 position : POSITION, uint foo : SV_Foo) : SV_Position
/*CHECK:
       ^^^^ no stage specified for entry point
       ^^^^ no stage specified for entry point 'main'; use either a '[shader("name")]' function attribute or the '-stage <name>' command-line option to specify a stage
*/
{
    return position;
}
