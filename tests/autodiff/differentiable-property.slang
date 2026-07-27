//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cuda

// A differentiability attribute on a property accessor must not crash semantic
// checking: declaring and reading such a getter compiles and runs. (#12210)
//
// This guards the front-end crash, not differentiation semantics — the property
// is read primally (never through fwd/bwd_diff), so the CHECK values verify only
// that ordinary property lowering is intact. Differentiating *through* a property
// accessor is tracked separately in #12232.

// Concrete struct with a [Differentiable] getter.
struct Square : IDifferentiable
{
    float v;
    property squared : float { [Differentiable] get { return v * v; } }
}

// A [Differentiable] setter, whose value parameter (newValue) is owned by the
// accessor — the path the crash-fix must leave intact.
struct Box : IDifferentiable
{
    float _v;
    property v : float
    {
        [Differentiable] get { return _v; }
        [Differentiable] set { _v = newValue; }
    }
}

// A [TreatAsDifferentiable] getter — a distinct differentiability-attribute
// synthesis branch that also reached the crashing code path.
struct Treated : IDifferentiable
{
    float v;
    property squared : float { [TreatAsDifferentiable] get { return v * v; } }
}

// An interface requirement `[Differentiable] get;` (a body-less accessor) reached
// the crash through a structurally distinct path — differentiability-requirement
// synthesis — when the property is used through a generic constrained to it.
interface ISquared : IDifferentiable
{
    property squared : float { [Differentiable] get; }
}
struct SquareImpl : ISquared
{
    float v;
    property squared : float { [Differentiable] get { return v * v; } }
}
float readThroughInterface<T : ISquared>(T s) { return s.squared; }

//TEST_INPUT:set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    Square s = {3.0};
    // CHECK: 9.0
    output[0] = s.squared;

    Box b = {4.0};
    b.v = 5.0;
    // CHECK: 5.0
    output[1] = b.v;

    Treated t = {2.0};
    // CHECK: 4.0
    output[2] = t.squared;

    SquareImpl si = {3.0};
    // CHECK: 9.0
    output[3] = readThroughInterface(si);
}
