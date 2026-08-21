//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
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
