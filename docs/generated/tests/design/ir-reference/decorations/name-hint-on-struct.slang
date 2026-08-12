//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#naming-and-provenance
//META: doc_section_digest=c55006c0bcfdbf481cbed2ad87133c84b45c1a360af8f94519a6b15522ec2953
//META: purpose=A user-declared struct carries a nameHint decoration whose string operand is the source identifier of the struct.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `nameHint` also names type declarations. The CHECK pins the
// decoration and then `struct %WidgetData`, showing the identifier is
// preserved on the type inst so later dumps and diagnostics can refer to
// the type by its source name.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

struct WidgetData
{
    int payload;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    WidgetData w;
    w.payload = int(tid.x);
    buf[0] = w.payload;
}

// CHECK: [nameHint("WidgetData")]
// CHECK: struct %WidgetData
