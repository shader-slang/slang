//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=Multiple module-scope `uniform VarDecl`s each lower to a `global_param` IR value, preserving source order and each carrying its declared type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: global-uniform "many" axis. The doc's row "VarDecl
// (global) -> IRGlobalVar" (spelled `global_param` for uniforms in
// the dump) must hold for any count. Three uniforms of distinct
// scalar / resource types each produce their own `let %x : T =
// global_param` declaration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int alpha;
uniform float beta;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = alpha + int(beta);
}

// CHECK: let  %alpha	: Int	= global_param
// CHECK: let  %beta	: Float	= global_param
// CHECK: let  %buf	: RWStructuredBuffer({{.*}})	= global_param
