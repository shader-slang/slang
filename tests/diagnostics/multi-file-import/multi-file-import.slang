// Test that multiple slang input files are treated as separate modules.
// When A.slang imports B.slang, and both are specified on the command line,
// the import should be deduplicated and not cause "ambiguous reference" errors.

//TEST:SIMPLE: tests/diagnostics/multi-file-import/module-b.slang tests/diagnostics/multi-file-import/module-a.slang -entry computeMain -stage compute
