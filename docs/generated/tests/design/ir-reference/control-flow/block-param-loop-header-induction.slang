//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#block-and-param
//META: doc_section_digest=2a352bcf0ce838b95cff334ee1235c003b2f526ca07b93710337b16c2a4872f8
//META: purpose=Boundary: a loop header carrying two independent induction variables receives two `param` declarations — one per induction value — and both initial values appear as trailing operands on the loop terminator.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A while-loop with a separately-declared induction variable plus an
// accumulator both arrive on the body header as block params; the
// loop terminator's trailing operands carry both initial values.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    int i = 0;
    while (i < n)
    {
        s = s + i;
        i = i + 1;
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int, 0 : Int)
// CHECK: param %i	: Int
// CHECK: param %s	: Int
