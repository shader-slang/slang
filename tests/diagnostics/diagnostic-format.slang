// Use FileCheck to check the text printed by the compiler. Tests using diag= automatically
// enable machine-readable output, so they would not exercise Visual Studio formatting.

// Select Visual Studio headers and indented source details.
//TEST:SIMPLE(filecheck=VS): -target hlsl -diagnostic-format vs

// VS output stays uncolored even when color is requested afterward.
//TEST:SIMPLE(filecheck=VS): -target hlsl -diagnostic-format vs -diagnostic-color always

// VS output also stays uncolored when color is requested first.
//TEST:SIMPLE(filecheck=VS): -target hlsl -diagnostic-color always -diagnostic-format vs

// Without a format option, preserve the normal rich diagnostic output.
//TEST:SIMPLE(filecheck=DEFAULT): -target hlsl

// The last format option wins: 'default' restores normal output after 'vs'.
//TEST:SIMPLE(filecheck=DEFAULT): -target hlsl -diagnostic-format vs -diagnostic-format default

// Machine-readable output takes precedence over the requested VS format.
//TEST:SIMPLE(filecheck=TSV): -target hlsl -diagnostic-format vs -enable-machine-readable-diagnostics

// Machine-readable output still takes precedence when the arguments are reversed.
//TEST:SIMPLE(filecheck=TSV): -target hlsl -enable-machine-readable-diagnostics -diagnostic-format vs

// Use VS formatting for errors while parsing subsequent command-line options.
//TEST:SIMPLE(filecheck=OPTION): -diagnostic-format vs -unknown-format-test-option

// Legacy diagnostics also use VS formatting, here for a missing -target argument.
//TEST:SIMPLE(filecheck=LEGACY): -diagnostic-format vs -target

// Reject unknown format names and report the supported values.
//TEST:SIMPLE(filecheck=INVALID): -diagnostic-format invalid

// Report a missing argument to -diagnostic-format.
//TEST:SIMPLE(filecheck=MISSING): -diagnostic-format

// VS: diagnostic-format.slang(10,13): error E30015: undefined identifier
// VS-NEXT: {{^ +}}10 | int value = unknownName;
// VS-NEXT: | ^^^^^^^^^^^ undefined identifier 'unknownName'.
// DEFAULT: error[E30015]: undefined identifier
// DEFAULT: --> diagnostic-format.slang:10:13
// TSV: E30015 error diagnostic-format.slang 10 13
// OPTION: command line: error E00017: unknown command-line option '-unknown-format-test-option'
// LEGACY: error E100001: expected an argument for command-line option '-target'
// INVALID: error[E00062]: unknown value for option '-diagnostic-format'. Valid values are 'default, vs'
// MISSING: expected an argument for command-line option '-diagnostic-format'

#line 10 "diagnostic-format.slang"
int value = unknownName;
