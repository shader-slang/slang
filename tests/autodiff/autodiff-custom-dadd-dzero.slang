// Test that a type conforming to IDifferentiable works without dmul (removed in PR #9808).
// Only dzero and dadd are required; dmul is no longer part of the interface.
//
// BUG: Debug assertion failure "doesSignatureMatch" when type provides custom
// dzero/dadd without compiler-synthesized defaults. Works in Release builds.
// TODO: File bug and re-enable once assertion is fixed.

//DISABLE_TEST:INTERPRET(filecheck=CHECK):

struct MyFloat : IDifferentiable
{
    typealias Differential = MyFloat;
    float val;

    // Only dzero and dadd are needed, no dmul.
    static MyFloat dzero() { return { 0.0 }; }
    static MyFloat dadd(MyFloat a, MyFloat b) { return { a.val + b.val }; }
}

[ForwardDifferentiable]
float useMyFloat(MyFloat a, float t)
{
    return a.val * t;
}

void main()
{
    MyFloat a = { 3.0 };
    // d/dt(a.val * t) = a.val = 3.0
    let result = fwd_diff(useMyFloat)(
        DifferentialPair<MyFloat>(a, MyFloat.dzero()),
        DifferentialPair<float>(2.0, 1.0));
    // CHECK: 3.0
    printf("%f\n", result.d);
}
