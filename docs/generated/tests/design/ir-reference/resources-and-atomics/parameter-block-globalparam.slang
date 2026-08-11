//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=3ebd10067ff5f6123adbd965e4471fda38311d82079a24af41d8faab3a3f45cc
//META: purpose=A `ParameterBlock<T>` module-scope parameter surfaces as a `global_param` with type `ParameterBlock(%T)`, and field accesses lower to `get_field_addr` on its `Ptr`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ParameterBlock<T>` is the parameter-grouping construct documented
// in the Shader IO row as another flavor of module-scope shader
// parameter; it shares the `global_param` opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Params { int4 vals; }
ParameterBlock<Params> pb;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    rwbuf[idx] = pb.vals[idx & 3];
}

// IR: %pb{{.*}}: ParameterBlock(%Params){{.*}}= global_param
// IR-LABEL: func %main
// IR: get_field_addr(%pb, %vals)
