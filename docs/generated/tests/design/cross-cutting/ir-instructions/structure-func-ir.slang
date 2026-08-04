//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=cdcc1238b1d73cfdad47a675356ee5e312b4cda22f2d9f5d255ae4ad1e0d4b21
//META: purpose=A source function declaration lowers to a `func` inst whose children are `block`s.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `func`/`IRFunc` structure row ("Function; children
// are blocks") together with the `block` control-flow row. The
// user-named helper `computeIt` appears as `func %computeIt` in the IR,
// and its body opens with a `block`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int computeIt(int x) { return x + x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = computeIt(a);
}

// IR: func %computeIt
// IR: block %{{.*}}
