//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#inlining-and-call-graph
//META: doc_section_digest=0c02024bf756789f5e2a8e8518524bff844c5a9c7658e631aab916c9a49f2d22
//META: purpose=Stress: eight different `[ForceInline]` callees (each called once) all inline into a single entry point — no callee name appears in the HLSL emit.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Many-different-callees axis on inline. Eight distinct
// `[ForceInline]` helpers each called once exercise the inliner's
// breadth dimension.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[ForceInline] int op0(int x) { return x; }
[ForceInline] int op1(int x) { return x + 1; }
[ForceInline] int op2(int x) { return x * 2; }
[ForceInline] int op3(int x) { return x - 3; }
[ForceInline] int op4(int x) { return x ^ 4; }
[ForceInline] int op5(int x) { return x | 5; }
[ForceInline] int op6(int x) { return x & 6; }
[ForceInline] int op7(int x) { return x + 7; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = a;
    v = op0(v) + op1(v) + op2(v) + op3(v) + op4(v) + op5(v) + op6(v) + op7(v);
    buf[0] = v;
}

// HLSL: void main
// HLSL-NOT: op0
// HLSL-NOT: op1
// HLSL-NOT: op2
// HLSL-NOT: op3
// HLSL-NOT: op4
// HLSL-NOT: op5
// HLSL-NOT: op6
// HLSL-NOT: op7
