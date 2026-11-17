//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#global-state
//META: doc_section_digest=b99cd634d7767c3e608ac1457ec02c747a3002b11e0c6ec01b999815f7b996c1
//META: purpose=A function-static const local lowers to a module-scope globalConstant carrying its value operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second AST origin of `globalConstant`: a `static const` declared
// inside a function body. CHECK requires the constant to surface as a
// module-scope `let %K : Int = globalConstant(7 : Int)` rather than as a
// function-local inst, which is the same treatment a module-scope `static
// const` gets.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

int scaled()
{
    static const int K = 7;
    return K * n;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = scaled();
}

// CHECK: [nameHint("K")]
// CHECK-NEXT: let  %K	: Int	= globalConstant(7 : Int)
