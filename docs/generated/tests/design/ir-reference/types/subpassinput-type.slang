//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=9bc7cd21838a70254771d6cf744cc574fc1ff6d4c25215b75c1badb8cf87e771
//META: purpose=A SubpassInput<T> uniform declared in a fragment entry point surfaces the SubpassInputType opcode parameterised by its element type in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A SubpassInput<T> uniform declared in a fragment entry point
// surfaces the SubpassInputType opcode parameterised by its element
// type in the IR dump. A fragment-shader global `SubpassInput<float4>`
// surfaces the `SubpassInputType` opcode on a `global_param` in the IR
// dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -stage fragment -entry main -dump-ir -o /dev/null

[[vk::input_attachment_index(0)]] SubpassInput<float4> inputColor;

float4 main() : SV_Target
{
    return inputColor.SubpassLoad();
}

// IR: let {{.*}}%inputColor{{.*}}: SubpassInputType{{.*}}= global_param
