//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#inlining-and-call-graph
//META: doc_section_digest=5916f676576778a4bdb5917c7cf917a7fca962142a97cb9c75be16910e0e08a2
//META: purpose=Stress: a `[ForceInline]` helper called 16 times is inlined at every call site — the callee name disappears from every observable target.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Sixteen call sites of `bumpBy` exercise the inliner's many-callsite
// behavior. Every site is rewritten in place; the callee name does
// not appear in HLSL or SPIR-V output.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[ForceInline]
int bumpBy(int x, int k) { return x + k; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = a;
    v = bumpBy(v, 1);
    v = bumpBy(v, 2);
    v = bumpBy(v, 3);
    v = bumpBy(v, 4);
    v = bumpBy(v, 5);
    v = bumpBy(v, 6);
    v = bumpBy(v, 7);
    v = bumpBy(v, 8);
    v = bumpBy(v, 9);
    v = bumpBy(v, 10);
    v = bumpBy(v, 11);
    v = bumpBy(v, 12);
    v = bumpBy(v, 13);
    v = bumpBy(v, 14);
    v = bumpBy(v, 15);
    v = bumpBy(v, 16);
    buf[0] = v;
}

// HLSL: void main
// HLSL-NOT: bumpBy
// SPIRV: OpEntryPoint
// SPIRV-NOT: bumpBy
