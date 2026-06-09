//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=d7cf9915ec0da5de567edb118ec898dd57bb3e7b5414db216254906018825537
//META: purpose=Boundary - a [loop] attribute on a for-loop produces loopControl with mode operand 1, the documented "loop" encoding distinct from [unroll]'s mode 0.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - the doc says loopControl's modeOperand is an
// IRConstant that records "unroll, loop, ...". The observed
// encoding is unroll=0, loop=1. The existing functional test
// uses a wildcard on the mode integer; this test pins the
// specific value for [loop].

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
