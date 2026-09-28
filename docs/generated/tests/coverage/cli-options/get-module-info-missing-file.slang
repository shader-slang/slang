//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -get-module-info diagnoses E00001 "cannot open file" and aborts the compile when its operand names a file that does not exist.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::GetModuleInfo` arm of
// `OptionsParser::_parse` in slang-options.cpp. The arm reads a path operand,
// opens it with `FileStream::init(..., FileMode::Open, FileAccess::Read, ...)`,
// and on failure diagnoses `Diagnostics::CannotOpenFile` against the synthetic
// command-line location before returning `SLANG_FAIL`. Nothing in either shipped
// suite reached this option at all.
//
// Only the cannot-open path is pinned here. The success path needs a serialized
// `.slang-module` produced by a prior slangc invocation, which a single-command
// SIMPLE test cannot stage; and the in-between case — a readable file that is not
// an IR blob — currently exits non-zero with no diagnostic at all, which is
// recorded as a finding rather than characterized here (see
// _meta/findings/cli-get-module-info-silent-failure.yaml).
//
// The diagnostic renders against the synthetic "command line" source, so the
// caret line quotes the whole argv; only the message and code are pinned.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -get-module-info no-such-module.slang-module

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
    b[0] = 1;
}

//CHECK: error[E00001]: cannot open file 'no-such-module.slang-module'
//CHECK: --> command line
