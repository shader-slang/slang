//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#naming-and-provenance
//META: doc_section_digest=c55006c0bcfdbf481cbed2ad87133c84b45c1a360af8f94519a6b15522ec2953
//META: purpose=A nameHint decoration also attaches to function parameters carrying the declared parameter name.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `nameHint` is not restricted to top-level symbols: the
// entry point's parameter gets one too. The CHECK pins the decoration and
// the `param %tid` line it precedes, which is the shape a backend relies on
// when it names the emitted variable after the source parameter.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [nameHint("tid")]
// CHECK: param %tid
