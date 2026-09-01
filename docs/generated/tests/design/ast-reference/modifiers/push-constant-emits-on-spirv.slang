//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#layout--binding-attributes
//META: doc_section_digest=937b78c13b95575b061eb7026bb075b7caeed29a419f54d8aaffc825020fc09c
//META: purpose=`[push_constant]` places the constant buffer in the SPIR-V PushConstant storage class instead of a descriptor binding.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the attribute changes where layout puts the parameter rather than
// being a documentation-only marker. CHECK asserts the emitted variable is
// declared `PushConstant`; an ordinary `ConstantBuffer` would be a `Uniform`
// variable with a descriptor binding instead.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

struct PC
{
    int k;
}

[push_constant]
ConstantBuffer<PC> pc;

RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main()
{
    outBuf[0] = pc.k;
}

//CHECK: OpVariable %{{[A-Za-z0-9_]+}} PushConstant
