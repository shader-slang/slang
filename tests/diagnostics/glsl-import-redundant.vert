//DIAGNOSTIC_TEST:SIMPLE(diag=CHK):

import glsl;
//CHK: ^^^^ redundant import of the builtin `glsl` module
//CHK: ^^^^ GLSL input already imports the builtin `glsl` module implicitly

void main()
{
    gl_Position = vec4(0.0);
}
