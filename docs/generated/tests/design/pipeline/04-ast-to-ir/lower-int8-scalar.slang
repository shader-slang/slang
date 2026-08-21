//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
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
