// `import glsl` does not change the source language, so using it from HLSL mixes the GLSL module's
// declarations and operator rules with HLSL parsing and semantics.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/import-glsl-in-hlsl.slang -Wno-118

import glsl;
/*CHECK:
       ^^^^ importing the builtin `glsl` module into HLSL source may introduce conflicting language semantics
       ^^^^ the module exposes GLSL declarations and operator rules without enabling GLSL syntax
*/

float useGlslIntrinsic(float value)
{
    return sin(value);
}
