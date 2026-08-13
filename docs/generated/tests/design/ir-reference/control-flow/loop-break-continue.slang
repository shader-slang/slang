//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-unconditional-branches
//META: doc_section_digest=a0f708a11d81970e17fbe525500eba453951d03287c41957fe027cbfb06d85f6
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
