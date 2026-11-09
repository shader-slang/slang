// `-allow-glsl` is normalized after all inputs are collected, so it selects actual GLSL even
// though this file's extension implies Slang.
// The second invocation also verifies that an explicit `-lang slang` conflict is diagnosed before
// the compatibility option takes precedence; the GLSL-only source below must still parse.

//DIAGNOSTIC_TEST:SIMPLE(diag=ALLOW): -allow-glsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CONFLICT): -allow-glsl -lang slang tests/diagnostics/deprecated-allow-glsl.slang -no-codegen
//ALLOW: `-allow-glsl` and `setAllowGLSLInput()` are deprecated and now treat every input translation unit as GLSL
//ALLOW-NOT: overrides an explicit slang source-language selection
//CONFLICT: `-allow-glsl` and `setAllowGLSLInput()` are deprecated and now treat every input translation unit as GLSL
//CONFLICT: the deprecated `AllowGLSL` compatibility option selects GLSL and overrides an explicit slang source-language selection on the same translation unit

layout(local_size_x = 1) in;

void main()
{}
