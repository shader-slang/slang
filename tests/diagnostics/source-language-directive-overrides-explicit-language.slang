// A source-content language directive takes precedence even over an explicitly requested
// language. The GLSL-only declaration below proves that the warning accompanies an effective
// parser-mode change rather than leaving the explicit Slang selection in force.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang slang tests/diagnostics/source-language-directive-overrides-explicit-language.slang -no-codegen

#version 450
//CHECK: source-language directive selects glsl and overrides the explicitly requested slang source language
//CHECK: the source directive selects glsl instead of slang

layout(local_size_x = 1) in;

void main()
{}
