//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A `(void)expr` cast evaluates its operand for side effects and yields the canonical void constant; no `CastToVoid` inst survives lowering.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the special case lowering applies to the builtin `__init(T)` on
// `void`: the operand call is still emitted (its side effect is preserved) but
// the cast itself does not become an instruction, so no back end has to know a
// cast-to-void opcode. The CHECKs pin `call %sideEffect` inside `main` and then
// assert that nothing spelled `castToVoid` appears between that call and the
// function's terminator.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int sideEffect(int x)
{
    return x + 1;
}

[numthreads(1, 1, 1)]
void main()
{
    (void)sideEffect(a);
    buf[0] = a;
}

// CHECK-LABEL: func %main
// CHECK: call %sideEffect
// CHECK-NOT: {{[cC]}}astToVoid
// CHECK: return_val(void_constant)
