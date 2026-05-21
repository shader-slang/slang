//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=094f3ccbff16c121504cf60cd04ae15b937c78df94da8d568013fe4104a433b9
//META: purpose=Expansion: RWStructuredBuffer parameterised by UInt8 names the narrow scalar as its first operand.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's RWStructuredBuffer row says the element-type operand
// can be any value type. A `RWStructuredBuffer<uint8_t>` shows
// `RWStructuredBuffer(UInt8, ...)` for the global_param.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint8_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint8_t(0xFF);
}

// IR: let {{.*}}%buf{{.*}}: RWStructuredBuffer(UInt8{{.*}}= global_param
