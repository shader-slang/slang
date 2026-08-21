//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=4b61bbe5577fdd277138f6c76e47bb415b4e0d9c6365f3fd1961205026f01739
//META: purpose=Boundary - a [loop] attribute on a for-loop produces loopControl with mode operand 1, the encoding distinct from [unroll]'s mode 0.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `modeOperand` of `loopControl` distinguishes the two
// loop-control attributes. This test pins 1 for `[loop]`; the `[unroll]`
// sibling pins 0. Pinning both endpoints is what makes the operand
// meaningful - a single wildcarded test could not tell the two modes
// apart.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    [loop]
    for (int i = 0; i < 4; i++) s += int(tid.x) * i;
    buf[0] = s;
}

// CHECK: [loopControl(1 : Int)]
