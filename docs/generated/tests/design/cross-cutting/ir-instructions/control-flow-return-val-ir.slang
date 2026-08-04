//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=0b68b77b6156c67db4c587635b263df5f1c7f5c3d702914334c92fc6b5e88c90
//META: purpose=A value-returning function terminates its block with the `return_val` terminator.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `return_val` terminator row. A helper function that
// returns a computed value ends its block with `return_val(%v)` in the
// IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

int addThem(int x, int y) { return x + y; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = addThem(a, b);
}

// IR: return_val(%{{.*}})
