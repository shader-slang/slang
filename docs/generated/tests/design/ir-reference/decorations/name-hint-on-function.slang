//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#namehint-namehintdecoration
//META: doc_section_digest=883eb56d4459110d6c60afc98e0feca78b8411f8e7783e41110d89c26b5c6c56
//META: purpose=A user-named function carries a nameHint decoration with the source identifier as a string literal operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says nameHint "Carries a human-readable name across IR
// passes". The dump shows it on the line immediately preceding the
// declared func opcode, with the source identifier as the string
// operand.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

int doubleIt(int x) { return x + x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = doubleIt(int(tid.x));
}

// CHECK: [nameHint("doubleIt")]
// CHECK: func %doubleIt
