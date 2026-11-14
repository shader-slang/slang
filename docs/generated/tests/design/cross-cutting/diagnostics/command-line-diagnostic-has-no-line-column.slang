//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=f952036c11ff9b8c1d73b32cb9eb2592525bdd2e44c5b243e4cfdec0a9352108
//META: purpose=A diagnostic raised while parsing the command line renders the path `command line` with no line:column, so it can only be pinned by its error code.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Command-line diagnostics are attached to a synthetic source rather
// than to a file in the translation unit, and the renderer special-cases
// that source: it prints the path alone. This test asks for a
// pass-through target that does not exist, which is rejected during
// option parsing, and asserts the location line is exactly `command line`
// with nothing following it - the CHECK-NOT for `command line:` is what
// rules out a decoded `line:column` suffix having been appended. The
// shader body compiles cleanly on its own, so the only diagnostic in the
// stream is the command-line one.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -pass-through bogusPassThroughTarget

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK-NOT: command line:
// CHECK: E00016
// CHECK: command line
// CHECK-NOT: command line:
