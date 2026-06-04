//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-unconditional-branches
//META: doc_section_digest=14af4d2a3a4636636d6099f23a462c390fb80ef7faa8966bf7a220163783dc45
//META: purpose=Boundary: a `break;` inside a nested inner loop targets the inner loop's breakLabel only — the outer loop's break/continue labels are unchanged and both `loop` terminators retain their independent structured-join operands.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two nested loops; the inner loop carries a `break` that exits only
// the inner loop. The LOWER-TO-IR pass emits two distinct `loop`
// terminators, each with its own (body, break, continue) labels, and
// the inner break's unconditionalBranch targets the inner loop's
// break label.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform int m;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            if (j > 3) break;
            s = s + 1;
        }
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int, 0 : Int)
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[a-zA-Z0-9_]+}}, 0 : Int)
// CHECK: cmpGT(%{{[a-zA-Z0-9_]+}}, 3 : Int)
// CHECK: unconditionalBranch(%{{[0-9]+}})
