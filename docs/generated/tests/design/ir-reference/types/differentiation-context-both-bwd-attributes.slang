//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/types.md#differentiation-types
//META: doc_section_digest=418e205c407dfedf73a453aa71e19d8c136a8759dd81549fefaf44a7781fd4e9
//META: purpose=A function carrying both [BackwardDifferentiable] and [BackwardDerivative(fn)] gets only the FromLegacy context opcode pair, not the BackwardDiff pair.
//META: intent=boundary
//META: pipeline_stage=lower-to-ir
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a function with both [BackwardDifferentiable] and [BackwardDerivative]
// gets a single IBackwardDifferentiable conformance, built from the FromLegacy
// opcode pair. CHECK asserts the FromLegacy entries are present and that the
// plain BackwardDiff pair never appears in the dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute
//TEST:SIMPLE(filecheck=ABSENT):-target spirv-asm -dump-ir -o - -entry main -stage compute

RWStructuredBuffer<float> outBuf;

void f_bwd(inout DifferentialPair<float> x, float dOut) { x = diffPair(x.p, 2.0 * x.p * dOut); }

[BackwardDifferentiable]
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

// CHECK-DAG: BackwardContextFromLegacyBwdDiffFunc(%f, %{{[a-zA-Z_0-9]+}})
// CHECK-DAG: BackwardMinimalContextFromLegacyBwdDiffFunc(%f, %{{[a-zA-Z_0-9]+}})
// ABSENT-NOT: BackwardDiffIntermediateContextType(%f)
// ABSENT-NOT: BackwardDiffMinimalContextType(%f)
