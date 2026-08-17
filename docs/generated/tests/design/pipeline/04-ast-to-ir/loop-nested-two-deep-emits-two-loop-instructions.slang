//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=Two nested ForStmts lower to two distinct `loop(...)` terminators inside `func %main`, each with its own induction-variable block parameter — sibling stress to the deep-if test.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: control-flow nesting axis (loop). With two nested
// `for`-loops the IR contains two `loop(...)` terminators inside the
// same function, each carrying its own induction variable as a block
// `param`. The doc claims `ForStmt` lowers to `loop` regardless of
// nesting.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int total = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            total += i + j;
        }
    }
    buf[0] = total;
}

// CHECK-LABEL: func %main
// CHECK: loop(
// CHECK: loop(
