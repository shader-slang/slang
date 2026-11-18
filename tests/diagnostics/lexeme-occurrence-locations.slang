//TEST:SIMPLE(filecheck=RICH,exit-code=2): -target hlsl
//TEST:SIMPLE(filecheck=TSV,exit-code=2): -target hlsl -enable-machine-readable-diagnostics

// Reusing an identifier's spelling must preserve each occurrence's filename, line, and column.
// We place the same undeclared name at different columns under separate `#line` mappings.

// RICH: result code = 2
// RICH-NEXT: standard error = {
// RICH-NEXT: error[E30015]: undefined identifier
// RICH-NEXT: --> occurrence-first.slang:101:13
// RICH-NEXT: |
// RICH-NEXT: 101 | int first = missingName;
// RICH-NEXT: | ^^^^^^^^^^^ undefined identifier 'missingName'.
// RICH-NEXT: ----'
// RICH-NEXT: error[E30015]: undefined identifier
// RICH-NEXT: --> occurrence-second.slang:307:18
// RICH-NEXT: |
// RICH-NEXT: 307 | int second = missingName;
// RICH-NEXT: | ^^^^^^^^^^^ undefined identifier 'missingName'.
// RICH-NEXT: ----'
// RICH-NEXT: }
// RICH-NEXT: standard output = {
// RICH-NEXT: }
// RICH-NOT: {{.}}

// TSV: result code = 2
// TSV-NEXT: standard error = {
// TSV-NEXT: E30015 error occurrence-first.slang 101 13 101 24 undefined identifier
// TSV-NEXT: E30015 span occurrence-first.slang 101 13 101 24 undefined identifier 'missingName'.
// TSV-NEXT: E30015 error occurrence-second.slang 307 18 307 29 undefined identifier
// TSV-NEXT: E30015 span occurrence-second.slang 307 18 307 29 undefined identifier 'missingName'.
// TSV-NEXT: }
// TSV-NEXT: standard output = {
// TSV-NEXT: }
// TSV-NOT: {{.}}

#line 101 "occurrence-first.slang"
int first = missingName;

#line 307 "occurrence-second.slang"
    int second = missingName;
