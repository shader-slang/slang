//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#loop
//META: doc_section_digest=11bb8e0156989c885bddf35636b284cd6a60a3e29d0c1d38b73c6df665c61b1f
//META: purpose=Boundary: a loop with a statically-known single iteration (`for (int i=0; i<1; i++)`) still emits the loop terminator with the three structured-join operands plus induction args.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge iteration bound: the loop body executes exactly once, but
// the LOWER-TO-IR pass still emits a full `loop` terminator. The
// loop is annotated with `[loopMaxIters(1)]` because the trip count
// is statically known.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    for (int i = 0; i < 1; i++)
    {
        s = s + n;
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: [loopMaxIters(1 : Int)]
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int, 0 : Int)
