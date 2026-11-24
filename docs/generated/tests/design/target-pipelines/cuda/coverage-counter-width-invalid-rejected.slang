//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#conditional-gates
//META: doc_section_digest=53c4b02712abd1606ada0cd075435fc703c872ab9fb03c55c715018c720c4b38
//META: purpose=Negative - slangc -trace-coverage-counter-width accepts only 32 or 64 and rejects any other value as E45113 on the command line, before the E45114 byte-width check inside linkAndOptimizeIR can be reached.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the front gate that makes the deeper byte-width check
// unreachable from slangc: the option parser only accepts the two
// documented bit widths, so a request of 16 fails on the command
// line rather than reaching the pass that would diagnose an invalid
// byte width. The CHECK pins the error code, which is the stable
// part of the message; the shader body is irrelevant because the
// option is rejected before any compilation happens.

//TEST:SIMPLE(filecheck=CHECK,exit-code=1):-target cuda -entry main -stage compute -trace-coverage -trace-coverage-counter-width 16

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = int(tid.x);
}

// CHECK: error{{.*}}E45113
// CHECK: accepts only `32` or `64`
