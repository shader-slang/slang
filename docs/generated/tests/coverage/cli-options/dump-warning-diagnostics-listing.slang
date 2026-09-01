//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -dump-warning-diagnostics prints the compiler's warning table as `<numeric id> : <name>` lines before the normal compile output.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::DumpWarningDiagnostics` arm of
// `OptionsParser::_parse` in slang-options.cpp, which calls
// `_dumpDiagnostics(Severity::Warning)`. That helper walks the diagnostic table
// and writes one `<id> : <name>` line per warning-severity entry, then parsing
// continues so the ordinary compile still happens.
//
// The full table is long and grows whenever a warning is added, so this pins the
// line format plus two stable, long-lived entries (`macro-redefinition` and
// `same-stage-specified-more-than-once`) rather than the whole listing, and then
// checks that the compile still produced HLSL afterwards. Both name strings were
// copied verbatim from a real slangc run.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -dump-warning-diagnostics

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
    b[0] = 1;
}

//CHECK: {{[0-9]+}} : same-stage-specified-more-than-once
//CHECK: {{[0-9]+}} : macro-redefinition
//CHECK: void main(
