//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=9bc7cd21838a70254771d6cf744cc574fc1ff6d4c25215b75c1badb8cf87e771
//META: purpose=A ConstantBuffer<T> uniform lowers to a global_param of IR type ConstantBuffer(<T>, <layout>, ...).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A ConstantBuffer<T> uniform lowers to a global_param of IR type
// ConstantBuffer(<T>, <layout>, ...). A `uniform
// ConstantBuffer<float>` shows up as `ConstantBuffer(Float,
// DefaultLayout, ...)` in the IR.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform ConstantBuffer<float> cb;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = cb;
}

// IR: let {{.*}}%cb{{.*}}: ConstantBuffer(Float{{.*}}= global_param
