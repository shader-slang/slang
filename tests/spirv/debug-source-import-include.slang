//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -g2 -emit-spirv-directly

// Regression guard for shader-slang/slang#11982 (the #include sub-case). Three physical source
// files participate here: this entry-point file, the module it `import`s
// (debug-source-import-include-module.slang), and the sidecar that module `#include`s
// (debug-source-import-include-sidecar.slang). Before the fix, the imported module's `#include`d
// sidecar got a second, orphaned DebugSource record (4 records for 3 files): the lazy
// getOrEmitDebugSource() producer hardcoded isIncludedFile=false and read content off disk, so it
// diverged from the per-source-file loop's source->isIncludedFile()/getContent() for #include'd
// files, defeating hoistable dedup. There must be exactly three DebugSource records, one per file.
//
// The count assertion runs alone against the whole module (its own CHECK prefix, one pass) and
// matches on the opcode/mnemonic only (no operands) so a record with a different operand shape
// cannot escape the count. The instruction-form prefix does not match the embedded -g2 source text
// of these directive lines, so it is immune to the embedded-source self-match trap.

import debug_source_import_include_module;

[shader("fragment")]
float4 main() : SV_Target
{
    return getIncludedValue();
}

// CHECK-COUNT-3: OpExtInst %void %{{[0-9]+}} DebugSource
// CHECK-NOT: OpExtInst %void %{{[0-9]+}} DebugSource
