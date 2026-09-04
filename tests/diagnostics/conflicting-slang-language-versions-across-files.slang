// A translation unit has one module-wide Slang language version. The diagnostic must identify the
// later conflicting directive and point back to the first primary file's selection.

//TEST:SIMPLE_EX(filecheck=CHECK): tests/diagnostics/conflicting-slang-language-versions-across-files.slang tests/diagnostics/conflicting-slang-language-versions-across-files-helper.slang -no-codegen

// CHECK: conflicting Slang language-version directives in one translation unit
// CHECK: conflicting-slang-language-versions-across-files-helper.slang:1:2
// CHECK: 1 | #language slang 2026
// CHECK-NEXT: {{ *}}|  ^^^^^^^^ this directive selects a different Slang language version
// CHECK: the translation unit's first Slang language-version directive is here
// CHECK: conflicting-slang-language-versions-across-files.slang:15:2
// CHECK: 15 | #language slang 2025
// CHECK-NEXT: {{ *}}|  --------

#language slang 2025
