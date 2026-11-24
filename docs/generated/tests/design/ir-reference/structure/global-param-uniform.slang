//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#global-state
//META: doc_section_digest=b99cd634d7767c3e608ac1457ec02c747a3002b11e0c6ec01b999815f7b996c1
//META: purpose=A module-scope uniform parameter lowers to a global_param opcode at module scope.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a shader parameter takes the `global_param` path rather than
// the `global_var` path. CHECK requires the module-scope line
// `let %gIn : Int = global_param`, which is also what shows the inst is the
// parameter's value: an ordinary `let` binding of type `Int`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int gIn;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = gIn;
}

// CHECK: let  %gIn	: Int	= global_param
