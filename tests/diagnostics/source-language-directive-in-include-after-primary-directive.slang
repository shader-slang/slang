// A primary content directive retains its source location. If a later semantic `__include`
// selects another language, E00121 must diagnose the included directive and point back here.

// DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// CHECK: source-language-directive-in-include-after-primary-directive-helper.slang:1:2
// CHECK: 1 | #version 450
// CHECK-NEXT: {{ *}}|  ^^^^^^^ this directive selects a different source language
// CHECK: source-language-directive-in-include-after-primary-directive.slang:12:2
// CHECK: 12 | #language 2026
// CHECK-NEXT: {{ *}}|  --------
#language 2026

module "source-language-directive-in-include-after-primary-directive";

__include "source-language-directive-in-include-after-primary-directive-helper.slang";
