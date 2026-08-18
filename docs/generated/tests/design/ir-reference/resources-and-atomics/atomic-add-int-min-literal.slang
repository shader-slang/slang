//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=e0b71e7cc30a0e069a5833b7fdc7038f02b6ab982bbba6656983e3a65dcba7ce
//META: purpose=`Atomic<int>::add(INT_MIN)` (smallest signed value, `-2147483648`) still lowers to the documented `atomicAdd` opcode; the negative literal is carried unchanged.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the smallest representable 32-bit signed value.
// The claim under test is only about opcode shape — nothing commits to
// runtime wrap-around — so the CHECK asserts that `-2147483648` reaches
// `atomicAdd` as an ordinary value operand and is neither rejected nor
// rewritten on the way down.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].add(-2147483648);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{[^,]+}}, {{.*}}-2147483648{{.*}})
