//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A WhileStmt lowers to a `loop` structured-branch terminator (sibling of for; doc lists `WhileStmt` in the same row as `ForStmt`).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `WhileStmt` alongside `ForStmt` as lowering to a
// structured branch. The `while` loop here produces a `loop(...)`
// terminator in IR — the same opcode as `for` — confirming the
// row's claim.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int i = 0;
    while (i < n)
    {
        buf[i] = i;
        i++;
    }
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{.*}}
