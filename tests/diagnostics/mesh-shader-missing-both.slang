//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0

// Test for #9444: mesh shader with neither [outputtopology] nor outputs.
// Both E38047 and E38048 should fire.

[shader("mesh")]
[numthreads(1, 1, 1)]
void main()
/*diag:
     ^^^^ mesh shader missing [outputtopology] attribute
     ^^^^ mesh shader entry point 'main' must have an '[outputtopology("point"|"line"|"triangle")]' attribute
     ^^^^ mesh shader missing OutputVertices/OutputIndices output
     ^^^^ mesh shader entry point 'main' must declare both an 'OutputVertices' and an 'OutputIndices' output parameter
*/
{
}
