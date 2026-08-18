//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#loop
//META: doc_section_digest=4246818bbad3a8ae8ca598731acc24e52393b997901fb8ec83554a8705bc12d4
//META: purpose=The back-edge of a loop is an unconditionalBranch back to the loop header carrying the updated induction values as argument operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// At the end of each loop iteration, the continue block branches
// back to the body header carrying the next iteration's induction
// values — both the running sum and the incremented index.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    for (int i = 0; i < n; i++)
    {
        s = s + i;
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: add(%i, 1 : Int)
// CHECK: unconditionalBranch(%{{[0-9]+}}, %{{[a-zA-Z0-9_]+}}, %{{[a-zA-Z0-9_]+}})
