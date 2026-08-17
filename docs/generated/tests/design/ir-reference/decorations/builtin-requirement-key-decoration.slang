//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#builtinrequirementdecoration
//META: doc_section_digest=54acdd20df2ce2d1b263de1808792a14a6fb098b1863cdbff8a8a473872ab568
//META: purpose=A built-in interface requirement is keyed by a builtinRequirementKey inst that carries a BuiltinRequirementDecoration naming the same requirement kind.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the role-keying claim: a recognized built-in requirement is not
// addressed by its position in the interface's requirement list but by a
// key that is deduplicated from, and tagged with, its kind. Conforming a
// struct to `IDifferentiable` pulls those requirements in. The CHECK
// captures the kind ordinal from the decoration and requires the very next
// line to be a `builtinRequirementKey` built from the same ordinal, which
// is what ties the tag to the key rather than to an arbitrary inst.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct Sample : IDifferentiable
{
    float value;
}

[Differentiable]
float square(Sample s)
{
    return s.value * s.value;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Sample s;
    s.value = float(tid.x);
    buf[0] = square(s);
}

// The captured ordinal must be the same on both lines: the decoration tags
// the key, and the key itself is constructed from that kind.
// CHECK: [BuiltinRequirementDecoration([[KIND:[0-9]+]] : Int)]
// CHECK-NEXT: {{.*}}= builtinRequirementKey([[KIND]] : Int)
