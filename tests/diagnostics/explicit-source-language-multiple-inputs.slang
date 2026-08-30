// An explicitly requested translation-unit language takes precedence over each primary file's
// extension. Diagnose the mismatching file regardless of the order in which the inputs appear.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang slang tests/diagnostics/explicit-source-language-multiple-inputs.slang tests/diagnostics/explicit-source-language-multiple-inputs.vert -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang slang tests/diagnostics/explicit-source-language-multiple-inputs.vert tests/diagnostics/explicit-source-language-multiple-inputs.slang -no-codegen

//CHECK: explicitly requested source language overrides the language implied by the extension of input file 'tests/diagnostics/explicit-source-language-multiple-inputs.vert'

void fromSlangFile() {}
