// Helper module for debug-import-location.slang.
// This file must not contain the entry point; it exists only to provide a
// function that the main shader imports so that the DebugCompilationUnit and
// source-file attribution can be tested.
module debug_import_helper;

public uint scale_val(uint x, uint factor)
{
    return x * factor;
}
