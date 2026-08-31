// A translation unit must diagnose content-level language selections that disagree across primary
// source files. The note must point back to the first directive, not merely to the first file that
// the parser happens to visit.

// DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK):
// tests/diagnostics/conflicting-source-language-directives-across-files.slang
// tests/diagnostics/conflicting-source-language-directives-across-files-helper.slang -no-codegen
// -Wno-120

// CHECK: conflicting-source-language-directives-across-files-helper.slang:1:2
// CHECK: 1 | #language 2026
// CHECK-NEXT: {{ *}}|  ^^^^^^^^ this directive selects a different source language
// CHECK: conflicting-source-language-directives-across-files.slang:16:2
// CHECK: 16 | #version 450
// CHECK-NEXT: {{ *}}|  -------
#version 450
