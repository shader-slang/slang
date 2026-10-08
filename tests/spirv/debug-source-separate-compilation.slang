// Regression guard for shader-slang/slang#11982 (separate-compilation sub-case, raised in review).
//
// Precompile a module to a .slang-module with -g2, then compile a consumer that references it with
// -r and imports it, emitting SPIR-V with -g2. The module's SourceFile is deserialized from the
// .slang-module and found in the consumer's source manager, but it carries no embedded content
// blob. getOrEmitDebugSource() must therefore fall back to loading the module off disk for the
// DebugSource source-text operand; otherwise the record degenerates to a filename-only
// "DebugSource %path" (dropping the text), which is the regression this test guards. The CHECK
// block below rejects any such filename-only record (see its own comment for how).

//TEST:COMPILE: tests/spirv/debug-source-separate-compilation-module.slang -g2 -o tests/spirv/debug-source-separate-compilation-module.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/spirv/debug-source-separate-compilation-module.slang-module -target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly

import "debug-source-separate-compilation-module";

[shader("fragment")]
float4 main() : SV_Target
{
    return getSeparatelyCompiledValue();
}

// Every DebugSource here must carry a source-text operand. The positive check ties the module's
// filename OpString id to a two-operand DebugSource; the CHECK-NOT then rejects ANY filename-only
// DebugSource (one operand, end-anchored with {{$}}), which is exactly the regression — before the
// fix the precompiled module's record degenerated to "DebugSource %file" with the text dropped, and
// the CHECK-NOT catches it whichever record it lands on. The OpExtInst instruction form does not
// appear inside the embedded -g2 source text, so it is immune to the embedded-source self-match trap.
// CHECK: [[MODULE_FILE:%[0-9]+]] = OpString "{{.*}}debug-source-separate-compilation-module.slang"
// CHECK: OpExtInst %void %{{[0-9]+}} DebugSource [[MODULE_FILE]] %{{[0-9]+}}
// CHECK-NOT: OpExtInst %void %{{[0-9]+}} DebugSource %{{[0-9]+}}{{$}}
