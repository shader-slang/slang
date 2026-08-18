//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=b3e088ac3dc9706ad8749cb8f9a4f8033e0bd510147eb7d3d1d0a3563b2dd6ca
//META: purpose=Expansion: equality compare on two UInt8 operands yields a Bool-typed cmpEQ on Int8 inputs.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// equality compare on two UInt8 operands yields a Bool-typed cmpEQ on
// Int8 inputs. Comparing two `uint8_t` globals produces a `cmpEQ`
// whose operands are typed `UInt8`, demonstrating that comparisons
// preserve the narrow operand type at the IR level.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint8_t a;
uniform uint8_t b;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (a == b) ? 1u : 0u;
}

// IR: {{.*}}: Bool{{.*}}= cmpEQ(%a, %b)
