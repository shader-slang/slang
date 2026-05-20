//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#terminators-unconditional-branches
//META: doc_section_digest=14af4d2a3a4636636d6099f23a462c390fb80ef7faa8966bf7a220163783dc45
//META: purpose=A `break;` lowers to unconditionalBranch to the loop's breakLabel; a `continue;` lowers to unconditionalBranch to the loop's continueLabel.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A for-loop body with both break and continue produces two
// unconditionalBranch terminators; the loop terminator's second and
// third operands are the break and continue labels they jump to.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    for (int i = 0; i < n; i++)
    {
        if (i == 5) break;
        if (i == 3) continue;
        s = s + i;
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int, 0 : Int)
// CHECK: unconditionalBranch(%{{[0-9]+}})
// CHECK: unconditionalBranch(%{{[0-9]+}}, %{{[a-zA-Z0-9_]+}})
