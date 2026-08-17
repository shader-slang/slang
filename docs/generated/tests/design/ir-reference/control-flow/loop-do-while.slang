//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#loop
//META: doc_section_digest=4246818bbad3a8ae8ca598731acc24e52393b997901fb8ec83554a8705bc12d4
//META: purpose=A do-while loop lowers to a loop terminator; the trailing test is implemented by negating the condition so the ifElse branches to break on true.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A do-while loop produces the same loop opcode. The test placement
// at the end inverts the condition: the lowering computes
// not(cond) and branches to break when true.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    int i = 0;
    do
    {
        s = s + i;
        i = i + 1;
    } while (i < n);
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int, 0 : Int)
// CHECK: cmpLT(%{{[a-zA-Z0-9_]+}}, %n)
// CHECK: not(%{{[0-9]+}})
