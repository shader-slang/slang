//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vectors-matrices-and-composite
//META: doc_section_digest=4be4c15ea47b77fa5c11493f84e33669d7c20c2d179310e3570700407b2ebc9f
//META: purpose=A unorm or snorm type modifier wraps the base type in an Attributed(baseType, attrs...) IR type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A type modifier such as unorm or snorm does not change the underlying
// scalar opcode; it wraps it, so the modifier survives as a separate
// attribute operand instead of being folded into a new scalar type. A
// struct declares one `unorm float` and one `snorm float` field; the
// field rows are FileChecked for `Attributed(Float, ...)`, which shows
// Float is still operand 0 and the modifier rides along as a trailing
// attribute operand.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage compute -entry main -dump-ir -o /dev/null

struct Norms
{
    unorm float u;
    snorm float s;
}

uniform Norms n;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = n.u + n.s;
}

// IR-LABEL: struct %Norms
// IR: field(%u, Attributed(Float,
// IR: field(%s, Attributed(Float,
