//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#global-state
//META: doc_section_digest=100b91a908217e68795cbc9dac7393e0325718d4c3df831fe3616ee38716a45a
//META: purpose=A module-scope static variable lowers to a global_var opcode with a Ptr-typed result and a one-block initializer body.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a module-scope mutable variable is addressed rather than
// valued: the inst's result type is `Ptr(Int)`, and its initializer is not
// folded into the declaration but lives in a child block that returns it.
// CHECK pins the `global_var %gCounter : Ptr(Int)` header and the
// `return_val(0 : Int)` that terminates the initializer block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

static int gCounter = 0;

[numthreads(1, 1, 1)]
void main()
{
    gCounter = gCounter + 1;
    buf[0] = gCounter;
}

// CHECK: global_var %gCounter	: Ptr(Int)
// CHECK: return_val(0 : Int)
