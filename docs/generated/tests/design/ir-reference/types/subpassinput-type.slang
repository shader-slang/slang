//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=4995d79f1d575606442d0357f0fb015ffcc2489a1b4116d09d5d6d1d0bde2e0f
//META: purpose=A SubpassInput<T> uniform declared in a fragment entry point surfaces the SubpassInputType opcode parameterised by its element type in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Resource and texture types" table lists
// `SubpassInputType` with operands
// `elementType: IRType, isMultisampleInst`. A fragment-shader
// global `SubpassInput<float4>` surfaces the `SubpassInputType`
// opcode on a `global_param` in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -stage fragment -entry main -dump-ir -o /dev/null

[[vk::input_attachment_index(0)]] SubpassInput<float4> inputColor;

float4 main() : SV_Target
{
    return inputColor.SubpassLoad();
}

// IR: let {{.*}}%inputColor{{.*}}: SubpassInputType{{.*}}= global_param
