//TEST:SIMPLE(filecheck=CHECK):-target spirv

// This test covers the human-readable (non-machine-readable) diagnostic path
// through formatDiagnostic() in slang-diagnostic-sink.cpp. Unlike tests that
// use diag=CHECK (which inject -enable-machine-readable-diagnostics and
// exercise diagnoseRichImpl / appendMacroExpansionNotes), this test does not
// inject that flag, so the "expanded from macro" note is emitted by the
// formatDiagnostic walker at ~line 458-512 of slang-diagnostic-sink.cpp.

#define DECLARE_VAR UndefinedType x;

void test()
{
    DECLARE_VAR
}

// CHECK: undefined identifier 'UndefinedType'
// CHECK: expanded from macro 'DECLARE_VAR'
