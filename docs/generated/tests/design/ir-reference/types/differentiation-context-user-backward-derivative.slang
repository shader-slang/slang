//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/types.md#differentiation-types
//META: doc_section_digest=2635ba0f0ff347f7bb12fca7370c73e78fa5912a1c4b89ffa316ebba8b10ecf0
//META: purpose=A [BackwardDerivative(fn)] primal function gets the FromLegacyBwdDiffFunc context opcode pair, which carries the user-written derivative function as a second operand.
//META: intent=functional
//META: pipeline_stage=lower-to-ir
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Which of the three context-channel families a function gets is decided at
// semantic-checking time by the differentiation attribute on the primal
// function: each arm of the checker's `IBackwardDifferentiable` extension
// synthesis sets the `irOp` of the two structs it synthesizes.
//
// The two opcodes are not alternatives — both structs are synthesized for every
// differentiable function and the witness table names both — so each test in
// this trio pins the pair, not one entry. One test per row of the attribute
// table, because the rows are separate checker arms rather than one rule.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

RWStructuredBuffer<float> outBuf;

void f_bwd(inout DifferentialPair<float> x, float dOut) { x = diffPair(x.p, 2.0 * x.p * dOut); }

[BackwardDerivative(f_bwd)]
float f(float x) { return x * x; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    var dp = diffPair(float(tid.x), 0.0);
    bwd_diff(f)(dp, 1.0);
    outBuf[0] = dp.d;
}

// Moving the derivative into a user-written function replaces the pair again,
// and these two opcodes take the derivative function as a second operand where
// the other two families take only the primal.
// CHECK-DAG: BackwardContextFromLegacyBwdDiffFunc(%f, %{{[a-zA-Z_0-9]+}})
// CHECK-DAG: BackwardMinimalContextFromLegacyBwdDiffFunc(%f, %{{[a-zA-Z_0-9]+}})
