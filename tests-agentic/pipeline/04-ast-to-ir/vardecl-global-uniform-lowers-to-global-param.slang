//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
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
