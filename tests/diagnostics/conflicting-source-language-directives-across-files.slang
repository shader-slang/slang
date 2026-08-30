// A translation unit must diagnose content-level language selections that disagree across primary
// source files. The note must point back to the first directive, not merely to the first file that
// the parser happens to visit.

// DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK):
// tests/diagnostics/conflicting-source-language-directives-across-files.slang
// tests/diagnostics/conflicting-source-language-directives-across-files-helper.slang -no-codegen
// -Wno-120

// CHECK: conflicting-source-language-directives-across-files-helper.slang
// CHECK: conflicting source-language directives in one translation unit
// CHECK: this directive selects a different source language
// CHECK: conflicting-source-language-directives-across-files.slang
// CHECK: the translation unit's first source-language directive is here

#version 450
