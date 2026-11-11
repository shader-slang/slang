//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=b56b816820af1c15450b34e955f5435969bcf8f2936c6dbb068f8ab21ec3dca3
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

//TEST:SIMPLE(filecheck=CHECK,exit-code=1):-target hlsl -entry main -stage compute -pass-through bogusPassThroughTarget

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
