//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=094f3ccbff16c121504cf60cd04ae15b937c78df94da8d568013fe4104a433b9
//META: purpose=A ConstantBuffer<T> uniform lowers to a global_param of IR type ConstantBuffer(<T>, <layout>, ...).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Resource and texture types" table lists
// `ConstantBuffer` (C++ wrapper `ConstantBufferType`) with operand
// `elementType: IRType, layoutType: IRType`. A
// `uniform ConstantBuffer<float>` shows up as
// `ConstantBuffer(Float, DefaultLayout, ...)` in the IR.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform ConstantBuffer<float> cb;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = cb;
}

// IR: let {{.*}}%cb{{.*}}: ConstantBuffer(Float{{.*}}= global_param
