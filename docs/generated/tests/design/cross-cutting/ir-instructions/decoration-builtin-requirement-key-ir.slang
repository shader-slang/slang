//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=af2eb87d563403d52d71a7d33122c279b70c033339e51393dbe00f9f5f506a59
//META: purpose=A built-in interface requirement key carries a `BuiltinRequirementDecoration` naming its kind and is produced by a `builtinRequirementKey` inst keyed on that same kind.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `BuiltinRequirementDecoration` row, which says the decoration
// tags an interface requirement key with its `BuiltinRequirementKind` so
// consumers find the requirement by role rather than by entry order. An
// `IDifferentiable` conformance is the surface that pulls built-in
// requirements (`Differential`, `dzero`, `dadd`, ...) into the module. The
// CHECKs assert the two halves appear together: a `builtinRequirementKey`
// inst whose single operand is the kind ordinal, and a
// `BuiltinRequirementDecoration` carrying that same ordinal on it. That
// pairing is what makes a lookup key-addressable by role.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct MyVal : IDifferentiable
{
    float x;
}

[Differentiable]
float square(MyVal v)
{
    return v.x * v.x;
}

uniform float a;
uniform RWStructuredBuffer<float> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    MyVal v;
    v.x = a;
    outbuf[0] = square(v);
}

// The captured ordinal ties the two lines together: the decoration on the
// key and the key's own operand must name the same requirement kind.
// IR: [BuiltinRequirementDecoration([[KIND:[0-9]+]] : Int)]
// IR-NEXT: {{.*}}= builtinRequirementKey([[KIND]] : Int)
