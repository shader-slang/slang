// A module-scope output counts as an output of the vertex shaders that own it, so the pedantic
// warning about a missing 'SV_Position' covers vertex shaders that write their outputs that way.
// GLSL vertex shaders write `gl_Position`, which the GLSL module declares as a builtin rather than
// a varying of the shader. For them, module-scope outputs can supply 'SV_Position' but do not by
// themselves trigger the warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=ON):-target spirv -entry main -stage vertex -Wpedantic -no-codegen -warnings-disable 30627
//DIAGNOSTIC_TEST:SIMPLE(diag=OFF):-target spirv -entry main -stage vertex -Wpedantic -no-codegen -warnings-disable 30627 -DWITH_POSITION
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=OFF):-lang glsl tests/diagnostics/vertex-missing-sv-position-global.slang -target spirv -entry main -stage vertex -Wpedantic -no-codegen -DUSE_GLSL
//DIAGNOSTIC_TEST:SIMPLE(diag=OFF):-target spirv -entry main -stage vertex -Wpedantic -no-codegen -warnings-disable 30627 -DUSE_IMPORT_GLSL
//DIAGNOSTIC_TEST:SIMPLE(diag=OFF):-target spirv -entry main -stage vertex -Wpedantic -no-codegen -warnings-disable 30627 -DUSE_INDIRECT_GLSL
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=OFF):-lang glsl tests/diagnostics/vertex-missing-sv-position-global.slang -target spirv -entry main -stage vertex -Wpedantic -no-codegen -DUSE_GLSL -DPARAM_OUTPUT -warnings-disable 38044

#if defined(USE_IMPORT_GLSL)
// Legacy Slang that imports the `glsl` module explicitly can write `gl_Position` too.
import glsl;
out float4 gColor;

void main()
{
    gl_Position = float4(0, 0, 0, 1);
    gColor = float4(1);
}
#elif defined(USE_INDIRECT_GLSL)
// A module that depends on the `glsl` module only through another import is treated the same way,
// because a function of the imported module can write `gl_Position` for it.
import "vertex-missing-sv-position-global-helper";
out float4 gColor;

void main()
{
    helperSetPosition();
    gColor = float4(1);
}
#elif !defined(USE_GLSL)
out float4 gColor;
#ifdef WITH_POSITION
out float4 gPosition : SV_Position;
#endif

void main()
/*ON:
     ^^^^ vertex shader 'main' has no output with the 'SV_Position' system value semantic
     ^^^^ vertex shader 'main' has no output with the 'SV_Position' system value semantic; if it feeds the rasterizer directly, the rasterizer will not receive valid vertex positions (add 'SV_Position' to a vertex output)
*/
{
    gColor = float4(1);
#ifdef WITH_POSITION
    gPosition = float4(0, 0, 0, 1);
#endif
}
#elif !defined(PARAM_OUTPUT)
// GLSL source: the shader writes `gl_Position`, so its module-scope output does not trigger the
// warning.
out vec4 color;

void main()
{
    gl_Position = vec4(0, 0, 0, 1);
    color = vec4(1);
}
#else
// GLSL source with an output parameter: the parameter counts as an output, and the module-scope
// output supplies 'SV_Position'.
out vec4 position : SV_Position;

void main(out vec4 color : COLOR)
{
    position = vec4(0, 0, 0, 1);
    color = vec4(1);
}
#endif
