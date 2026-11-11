//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=3ea14a1946d125d6e51f1168520ddc4e04afd09127febe10b5079af9e30b1816
//META: purpose=-notes-disable requires the named diagnostic to be a note, so naming an error id is rejected as unknown.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies -notes-disable rejects a diagnostic that is not a Note. 39999 is an
// error entry, so the option reports E31111 (unknown diagnostic) and exits with
// the command-line error code 1.

//TEST:SIMPLE(filecheck=CHECK,exit-code=1):-target hlsl -entry main -stage compute -notes-disable 39999

void f(int a) {}
void f(float a, int b) {}

[numthreads(1, 1, 1)]
void main()
{
    f("x", 1, 2);
}

// CHECK: E31111
