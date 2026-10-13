//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=73a8b7d64d9596c97c899fba163502910f89289de9adea3eac1da3e0599596e6
//META: purpose=Structured branches (`ifElse`) encode the join (merge) point as an explicit operand on the terminator; the two branches reach it via `unconditionalBranch`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says "structured branches whose join point is an explicit
// operand on the terminator". For `ifElse` the four operands are
// `(cond, %then, %else, %merge)`; after the then- and else-blocks
// finish writing to the buffer, both branches end with
// `unconditionalBranch(%merge)`. We verify both halves: the
// `ifElse` terminator and the `unconditionalBranch` join.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (a > 0)
        buf[0] = 1;
    else
        buf[0] = 2;
}

// CHECK-LABEL: func %main
// CHECK: ifElse(%{{.*}}, %{{.*}}, %{{.*}}, %{{.*}})
// CHECK: unconditionalBranch(%{{.*}})
// CHECK: unconditionalBranch(%{{.*}})
