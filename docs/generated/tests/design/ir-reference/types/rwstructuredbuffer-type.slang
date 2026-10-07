//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=18390b63de1003330aaf1c938cc3b9a195ccaebbb584bbb2edb3db5df8f75c02
//META: purpose=A RWStructuredBuffer<T> uniform lowers to a global_param of IR type RWStructuredBuffer(<T>, <layout>, ...).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A RWStructuredBuffer<T> uniform lowers to a global_param of IR type
// RWStructuredBuffer(<T>, <layout>, ...). A `uniform
// RWStructuredBuffer<float>` declaration surfaces
// `RWStructuredBuffer(Float, DefaultLayout, ...)` as the IR type of
// its `global_param`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<float> buf;
uniform float a;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a;
}

// IR: let {{.*}}%buf{{.*}}: RWStructuredBuffer(Float{{.*}}= global_param
