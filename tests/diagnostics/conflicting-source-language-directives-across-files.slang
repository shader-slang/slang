// A translation unit must diagnose content-level language selections that disagree across primary
// source files. The note must point back to the first directive, not merely to the first file that
// the parser happens to visit. E00120 is intentionally suppressed because the test is about the
// conflict between the two directives, not the first directive overriding the `.slang` extension.

//TEST:SIMPLE_EX(filecheck=CHECK): tests/diagnostics/conflicting-source-language-directives-across-files.slang tests/diagnostics/conflicting-source-language-directives-across-files-helper.slang -no-codegen -Wno-120

// CHECK: conflicting-source-language-directives-across-files-helper.slang:1:2
// CHECK: 1 | #language 2026
// CHECK-NEXT: {{ *}}|  ^^^^^^^^ this directive selects a different source language
// CHECK: conflicting-source-language-directives-across-files.slang:14:2
// CHECK: 14 | #version 450
// CHECK-NEXT: {{ *}}|  -------
#version 450
