// A source-content language directive takes precedence even over an explicitly requested
// language. The GLSL-only declaration below proves that the warning accompanies an effective
// parser-mode change rather than leaving the explicit Slang selection in force.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -lang slang -no-codegen

#version 450
//CHECK: source-language directive overrides the language selected for this input
//CHECK: the source directive takes precedence over the requested or file-extension-implied language

layout(local_size_x = 1) in;

void main()
{}
