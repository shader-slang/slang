//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#namehint-namehintdecoration
//META: doc_section_digest=883eb56d4459110d6c60afc98e0feca78b8411f8e7783e41110d89c26b5c6c56
//META: purpose=Two distinct user-named functions each carry their own nameHint decoration with the matching source identifier as the string operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc: "Pass authors should preserve nameHint on results
// when rewriting an inst — losing it costs debuggability for no
// IR correctness benefit." Confirm the decoration's identifier
// passthrough is exact for multiple co-existing user names.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

int alpha(int x) { return x + 1; }
int beta(int x)  { return x * 2; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = alpha(int(tid.x)) + beta(int(tid.x));
}

// CHECK-DAG: [nameHint("alpha")]
// CHECK-DAG: [nameHint("beta")]
