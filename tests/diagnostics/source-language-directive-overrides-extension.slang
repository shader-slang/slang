// A source-content language directive takes precedence over the language implied by the file
// extension, but the inconsistency should not be silent.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#version 450
//CHECK: source-language directive selects glsl and overrides the file-extension-implied slang source language
//CHECK: the source directive selects glsl instead of slang

void main()
{}
