//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): tests/diagnostics/self-import.slang -stage compute

// This test compiles two Slang shaders via the command line, where the second
// shader imports this one. This should produce a warning.

interface I { }

// CHECK: warning 38204: module 'self-import-extra' imported more than once
