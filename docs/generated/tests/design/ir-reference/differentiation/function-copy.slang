//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#synthesized-derivative-witnesses
//META: doc_section_digest=d0d372acea2ddd87761f403b676bd55520cc30b10f78ab301fc5cad68440e053
//META: purpose=FunctionCopy names an existing user function as the body of a synthesized derivative member, taking that function as its single operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a user-written reverse-mode derivative attached with
// `[BackwardDerivative(myBwd)]` does not become the derivative member directly
// but is wrapped: the synthesized `h.bwd_diff` member is a `FunctionCopy` whose
// operand is `%myBwd`. CHECK pins the member's name hint on the preceding line
// and then requires the producing opcode and its single operand, plus the
// combined reverse-mode signature that the member has to present regardless of
// how `myBwd` itself was declared.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

void myBwd(inout DifferentialPair<float> dpx, float dOut)
{
    dpx = DifferentialPair<float>(dpx.p, 2.0 * dpx.p * dOut);
}

[BackwardDerivative(myBwd)]
float h(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    DifferentialPair<float> dp = DifferentialPair<float>(a, 0.0);
    __bwd_diff(h)(dp, 1.0);
    buf[0] = dp.d;
}

// CHECK: nameHint("h.bwd_diff")
// CHECK-NEXT: Func(Void, BorrowInOutParam(DiffPair(Float, {{%[A-Za-z0-9_]+}})), Float) = FunctionCopy(%myBwd)
