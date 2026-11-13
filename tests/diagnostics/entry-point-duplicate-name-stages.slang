// SPIR-V keeps one entry-point namespace per execution model, so a vertex and a fragment shader
// can both be named `Main` there (shader-slang/slang#14053). Metal and WGSL put every entry
// point into one namespace, so the same pair is an error on those targets. GLSL names every entry
// point `main`, so names are not compared there; a whole-program SPIR-V compile through GLSL is
// rejected by glslang instead. The LLVM emitter writes a named symbol only for a compute entry
// point, so a vertex and a fragment `Main` keep their mangled names there and do not collide.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -whole-program
//DIAGNOSTIC_TEST:SIMPLE(filecheck=VIA_GLSL,exit-code=-1):-target spirv-asm -emit-spirv-via-glsl -whole-program
//TEST:SIMPLE(filecheck=LLVM):-target llvm-ir -whole-program
//DIAGNOSTIC_TEST:SIMPLE(filecheck=AMBIG,exit-code=-1):-target spirv -entry Main -stage fragment

// SPIRV-DAG: OpEntryPoint Vertex %{{.*}} "Main"
// SPIRV-DAG: OpEntryPoint Fragment %{{.*}} "Main"

// `-entry` names one function, so `-stage fragment` does not pick one of the two `Main`.
// AMBIG: error[E38017]

// LLVM-NOT: E38016
// LLVM: define {{.*}}Main

// VIA_GLSL-NOT: E38016
// VIA_GLSL: E00101
// VIA_GLSL-NOT: E38016

[shader("vertex")]
float4 Main(float4 position: POSITION) : SV_Position
/*CHECK:
       ^^^^ the other entry point emitted as 'Main' is here
*/
{
    return position;
}

[shader("fragment")]
float4 Main(float3 color: COLOR) : SV_Target
/*CHECK:
       ^^^^ more than one entry point would be emitted as 'Main' for target
       ^^^^ this entry point would also be emitted as 'Main'
*/
{
    return float4(color, 1);
}
