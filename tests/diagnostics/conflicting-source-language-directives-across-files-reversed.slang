// Reverse the primary-source order to prove that the conflict note follows the first directive,
// rather than being anchored to a particular file or diagnostic traversal order. E00120 is
// intentionally suppressed because the first directive's extension mismatch is not under test.

//TEST:SIMPLE_EX(filecheck=CHECK): tests/diagnostics/conflicting-source-language-directives-across-files-helper.slang tests/diagnostics/conflicting-source-language-directives-across-files.slang -no-codegen -Wno-120

// CHECK: conflicting-source-language-directives-across-files.slang:14:2
// CHECK: 14 | #version 450
// CHECK-NEXT: {{ *}}|  ^^^^^^^ this directive selects a different source language
// CHECK: conflicting-source-language-directives-across-files-helper.slang:1:2
// CHECK: 1 | #language 2026
// CHECK-NEXT: {{ *}}|  --------
