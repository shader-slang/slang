//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T20:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A module-scope `uniform int8_t` lowers with IR type `Int8`; the doc's "VarDecl(global) -> IRGlobalVar" row at the narrow-integer end of the type axis.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: storage axis for the global-VarDecl row, exercising
// `int8_t` as the scalar element type of a `global_param`.
// `uniform` defeats constant folding so the load survives to IR.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int8_t a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(a);
}

// CHECK: [nameHint("a")]
// CHECK: let {{.*}}a{{.*}}: Int8{{.*}}= global_param
