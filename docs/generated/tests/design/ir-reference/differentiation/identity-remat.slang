//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#synthesized-derivative-witnesses
//META: doc_section_digest=d0d372acea2ddd87761f403b676bd55520cc30b10f78ab301fc5cad68440e053
//META: purpose=A user-provided __apply whose minimal context is its BwdCallable gets a remat member built from IdentityRemat over the apply_bwd member.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one row whose AST origin is a user-written `__apply` function
// extension. Because the context type the user returns is both the callable and
// the minimal context, rematerialization has nothing to recompute and the
// synthesized `remat` member is the identity. CHECK binds the `apply_bwd`
// member — itself a `FunctionCopy` of the user's `__apply` body — and then
// requires `remat` to be `IdentityRemat` over that same value, with a signature
// whose full-context and minimal-context parameters are the same user type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -experimental-feature

uniform float a;
uniform RWStructuredBuffer<float> buf;

float sqr(float x) { return x * x; }

struct SqrContext
{
    float cached;
    void operator()(out float dx, float dOut) { dx = dOut * cached; }
};

__func_extension __apply(sqr)(float x) -> Tuple<float, SqrContext>
{
    return makeTuple(x * x, SqrContext(2.0 * x));
}

[numthreads(1, 1, 1)]
void main()
{
    var dpx = diffPair(a, 0.0);
    bwd_diff(sqr)(dpx, 1.0);
    buf[0] = dpx.d;
}

// CHECK: nameHint("sqr.apply_bwd")
// CHECK-NEXT: let [[APPLY:%[A-Za-z0-9_]+]] : Func(tuple_type(Float, %SqrContext), Float) = FunctionCopy(
// CHECK: nameHint("sqr.remat")
// CHECK-NEXT: Func(%SqrContext, %SqrContext, Float) = IdentityRemat([[APPLY]])
