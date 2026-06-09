//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#linkage-and-lifetime
//META: doc_section_digest=51a07b0aba2f9f7d1b6cdf9777d9f2f83e7fbd3758ac6f0a6e034ab8f5d4454b
//META: purpose=A top-level struct declaration carries an export decoration recording its mangled type-linkage name (prefix _ST).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The export decoration applies not only to functions but to
// every top-level user declaration; structs receive an export
// decoration whose mangled name begins with the type-namespace
// prefix `_ST`.

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

// CHECK: [export("_ST{{.*}}WidgetData{{.*}}")]
// CHECK: [nameHint("WidgetData")]
// CHECK: struct %WidgetData
