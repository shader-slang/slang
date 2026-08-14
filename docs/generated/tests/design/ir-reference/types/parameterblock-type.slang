//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=9bc7cd21838a70254771d6cf744cc574fc1ff6d4c25215b75c1badb8cf87e771
//META: purpose=A ParameterBlock<T> global lowers to a global_param whose IR type is ParameterBlock(<T>).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ParameterBlock is its own type opcode rather than a spelling of
// ConstantBuffer, and its element type is the first (and here only)
// operand. A module-scope `ParameterBlock<Stuff> pb` is read in the
// kernel so the parameter survives DCE; the dump is FileChecked for the
// named global_param carrying `ParameterBlock(%Stuff)`, which shows the
// user struct's IR identity sits in the element-type slot.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage compute -entry main -dump-ir -o /dev/null

struct Stuff { float4 a; }

ParameterBlock<Stuff> pb;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = pb.a.x;
}

// IR: let %pb{{.*}}: ParameterBlock(%Stuff){{.*}}= global_param
