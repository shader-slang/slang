//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T20:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A `ParameterBlock<Narrow>` over a struct of narrow types lowers to a `global_param` whose IR type is `ParameterBlock(%Narrow)`; the inner struct's `field(...)` rows preserve each narrow type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: composition axis — ParameterBlock around a struct
// whose fields are narrow types. Verifies the
// `ParameterBlock(%T)` IR-side spelling for the block itself and
// the per-field IR types for the wrapped struct.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Narrow { int8_t i8; int16_t i16; half h; }

ParameterBlock<Narrow> pb;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(pb.i8) + int(pb.i16) + int(pb.h);
}

// CHECK-LABEL: struct %Narrow
// CHECK: field({{.*}}, Int8)
// CHECK: field({{.*}}, Int16)
// CHECK: field({{.*}}, Half)
// CHECK: [nameHint("pb")]
// CHECK: let {{.*}}pb{{.*}}: ParameterBlock(%Narrow){{.*}}= global_param
