// A source-content language directive takes precedence over the language implied by the file
// extension, but the inconsistency should not be silent.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#version 450
//CHECK: source-language directive overrides the language selected for this input
//CHECK: the source directive takes precedence over the requested or file-extension-implied language

void main()
{}
