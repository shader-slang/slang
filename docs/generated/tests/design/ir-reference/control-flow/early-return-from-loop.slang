//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-returns-and-yields
//META: doc_section_digest=010f71e10305e8452b6909c94e2da52806525700f6c78aa6daffe80e8b4ca211
//META: purpose=An early-return inside a loop body produces a return_val terminator on a block that is not the function's natural exit — return is a per-block terminator independent of loop structure.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A helper that returns early from inside a loop body emits a
// return_val inside the loop body block — the loop terminator is
// elsewhere; return is a terminator on the block it ends.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int target;
uniform int n;
uniform RWStructuredBuffer<int> buf;

int findIndex(int t, int upper)
{
    for (int i = 0; i < upper; i++)
    {
        if (i == t)
            return i;
    }
    return -1;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = findIndex(target, n);
}

// CHECK-LABEL: func %findIndex
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int)
// CHECK: return_val(%{{[a-zA-Z0-9_]+}})
// CHECK: return_val(%{{[0-9]+}})
