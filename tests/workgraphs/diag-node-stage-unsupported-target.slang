//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV,non-exhaustive): -target spirv-asm -stage node -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=GLSL,non-exhaustive): -target glsl -stage node -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=METAL,non-exhaustive): -target metal -stage node -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=WGSL,non-exhaustive): -target wgsl -stage node -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CPP,non-exhaustive): -target cpp -stage node -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CUDA,non-exhaustive): -target cuda -stage node -entry main

// Test that node-stage shaders fail cleanly on targets without Work Graph support.

[shader("node")]
[NodeLaunch("thread")]
void main()
{
}

//SPIRV: E31405
//GLSL: E31405
//METAL: E31405
//WGSL: E31405
//CPP: E31405
//CUDA: E31405
