//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=c0e97396ea139ceab54212053b6d443692d19f0faf2847c4dc505373ae8c4861
//META: purpose=A diagnostic whose location falls in a synthesized token-paste view renders that view's name and a line:column inside it.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the token-paste boundary of location decoding: the offending token
// exists only in a view synthesized by `##`, not in any file on disk. A macro
// pastes two fragments into an undeclared name; CHECK asserts the rendered
// location names the token-paste view and that the pasted spelling appears in
// the message, so the sink resolved a SourceLoc that has no on-disk backing.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

#define CAT(a, b) a##b

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = CAT(undefined, Thing);
}

// CHECK: E30015
// CHECK: token paste
// CHECK: undefinedThing
