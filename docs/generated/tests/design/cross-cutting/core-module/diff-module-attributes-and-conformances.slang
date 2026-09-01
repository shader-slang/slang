//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=261236714a826217dc94815d8118cf46f225066afa1514d96e19894b7a9f81e2
//META: purpose=The diff meta-module supplies the derivative attribute syntax ([ForwardDerivative], [BackwardDerivative]), the NullDifferential type, and IDifferentiable conformances for Array, Optional and Tuple.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the differentiation surface the diff meta-module owns is in
// scope with no import. Two user-written derivative functions are
// attached with [ForwardDerivative] and [BackwardDerivative] and then
// invoked through fwd_diff / bwd_diff, so the CHECKed 7.0 and 3.0 can
// only appear if the attributes bound the user functions instead of
// the compiler synthesizing derivatives. The same line also
// instantiates an IDifferentiable-constrained generic at Array,
// Optional and Tuple and constructs a NullDifferential, covering the
// conformances and the type the module declares.

//TEST:INTERPRET(filecheck=CHECK):

DifferentialPair<float> myFwd(DifferentialPair<float> x)
{
    return diffPair(x.p * x.p, float(7.0) * x.d);
}

[ForwardDerivative(myFwd)]
float square(float x) { return x * x; }

void myBwd(inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, float(3.0) * dOut);
}

[BackwardDerivative(myBwd)]
float cube(float x) { return x * x * x; }

int conforms<T : IDifferentiable>(T v) { return 1; }

void main()
{
    var fwd = fwd_diff(square)(diffPair(4.0, 1.0));
    var dp = diffPair(2.0, 0.0);
    bwd_diff(cube)(dp, 1.0);

    float arr[3] = { 1, 2, 3 };
    Optional<float> opt = 1.0f;
    Tuple<float, float> tup = makeTuple(1.0f, 2.0f);
    NullDifferential nd = NullDifferential();

    // CHECK: fwd=7.0 bwd=3.0 arr=1 opt=1 tup=1 null=0
    printf("fwd=%.1f bwd=%.1f arr=%d opt=%d tup=%d null=%d\n",
           fwd.d, dp.d, conforms(arr), conforms(opt), conforms(tup), int(nd.dummy));
}
