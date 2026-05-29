//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=094f3ccbff16c121504cf60cd04ae15b937c78df94da8d568013fe4104a433b9
//META: purpose=ByteAddressBuffer and RWByteAddressBuffer uniforms lower to global_params of the same-spelled IR types (no element-type operand).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Resource and texture types" table lists
// `ByteAddressBuffer` and `RWByteAddressBuffer` as opcodes with no
// element-type operand (the table's Operand column is "—"). Both
// uniforms surface as bare-named IR types on their respective
// `global_param` instructions.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform ByteAddressBuffer src;
uniform RWByteAddressBuffer dst;

[numthreads(1, 1, 1)]
void main()
{
    dst.Store(0, src.Load(0));
}

// IR: let {{.*}}%src{{.*}}: ByteAddressBuffer{{.*}}= global_param
// IR: let {{.*}}%dst{{.*}}: RWByteAddressBuffer{{.*}}= global_param
