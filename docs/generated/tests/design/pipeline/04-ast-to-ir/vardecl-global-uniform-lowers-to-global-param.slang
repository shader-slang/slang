//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A module-scope uniform VarDecl lowers to an `IRGlobalVar` (spelled `global_param` in the IR dump).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says global `VarDecl` → `IRGlobalVar`. In shader code the
// usual surface for that is a `uniform` declaration, which lowers to
// `global_param` in the IR dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int myUniformInt;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = myUniformInt;
}

// CHECK: [nameHint("myUniformInt")]
// CHECK: let {{.*}}myUniformInt{{.*}}: Int{{.*}}= global_param
