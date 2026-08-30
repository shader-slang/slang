// `-allow-glsl` is normalized after all inputs are collected, so it selects actual GLSL even
// though this file's extension implies Slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -allow-glsl -no-codegen
//CHECK: `-allow-glsl` and `setAllowGLSLInput()` are deprecated and now treat every input translation unit as GLSL
//CHECK: explicitly requested source language overrides the language implied by the extension of input file 'tests/diagnostics/deprecated-allow-glsl.slang'

#version 450

layout(local_size_x = 1) in;

void main()
{}
