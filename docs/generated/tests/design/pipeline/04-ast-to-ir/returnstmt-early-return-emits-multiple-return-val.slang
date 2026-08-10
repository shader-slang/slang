//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=Multiple ReturnStmts in a single FuncDecl each lower to their own `return_val(...)` terminator inside distinct basic blocks, all of which terminate the same IRFunc.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: return-shape axis — early return / multiple returns.
// The doc's row "ReturnStmt -> IRReturn terminator" applies once per
// source-level `return`. With three return statements the IRFunc
// body contains three distinct `return_val(...)` terminators in
// separate blocks.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int clamp01(int v)
{
    if (v < 0) return -1;
    if (v > 100) return 100;
    return v;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = clamp01(a);
}

// CHECK-LABEL: func %clamp01
// CHECK: return_val(-1 : Int)
// CHECK: return_val(100 : Int)
// CHECK: return_val(%{{.*}})
