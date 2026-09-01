//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -dump-module <path> loads the named file as a module and prints its IR disassembly, including the entry-point decoration and function body of the named entry point.
//META: intent=characterization
//META: characterization-unverified=true
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::DumpModule` arm of `OptionsParser::_parse`
// in slang-options.cpp — nearly ninety uncovered lines. The arm spins up a fresh
// `slang::ISession`, tries `loadModuleFromSourceString` on the file's bytes and
// falls back to `loadModuleFromIRBlob`, then calls `IModule::disassemble` and
// writes the result out through `m_sink->diagnoseRaw(Severity::Note, ...)`.
//
// The operand does not have to be a serialized `.slang-module`: a plain `.slang`
// source file takes the `loadModuleFromSourceString` branch, which is what this
// test uses so that it stays a single self-contained slangc invocation. The
// operand is spelled as a repository-root-relative path because slang-test runs
// slangc from the repository root.
//
// The disassembly text is verbose and full of generated ids, so the CHECKs pin
// the structural landmarks that identify it as module IR — the `entryPoint`
// decoration carrying the entry name, the `numThreads` decoration matching the
// attribute below, and the `rwstructuredBufferGetElementPtr` for `b[0] = 1`.
// Ids, mangled export names and types are wildcarded.
//
// Tagged characterization-unverified: `-dump-module` is a debugging aid with no
// specified output format, so the exact disassembly spelling is not contractual.

//TEST:SIMPLE(filecheck=CHECK):-dump-module docs/generated/tests/coverage/cli-options/dump-module-ir-disassembly.slang -target hlsl -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
    b[0] = 1;
}

//CHECK: [entryPoint({{[0-9]+}} : Int, "main", "path")]
//CHECK-NEXT: [numThreads(1 : Int, 1 : Int, 1 : Int)]
//CHECK: func %main
//CHECK: rwstructuredBufferGetElementPtr
