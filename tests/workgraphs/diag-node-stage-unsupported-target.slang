//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV,non-exhaustive): -target spirv-asm -stage node -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=GLSL,non-exhaustive): -target glsl -stage node -entry main

// Test that node-stage shaders fail cleanly on targets without Work Graph support.

[shader("node")]
[NodeLaunch("thread")]
void main()
{
}

//SPIRV: E31405
//GLSL: E31405
