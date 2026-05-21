//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A FuncDecl with zero parameters lowers to an IRFunc whose entry block has no `param` instructions; the function type carries only the return type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: function-parameter "zero" edge — sibling of the
// many-params (8) test. With no parameters the IRFunc's type is
// `Func(Int)` (return-only) and the entry block has zero `param`
// rows. The doc's row "FuncDecl -> IRFunc containing one or more
// IRBlocks" still holds.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

int nullary() { return 7; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = nullary();
}

// CHECK-LABEL: func %nullary	: Func(Int)
// CHECK: return_val(7 : Int)
