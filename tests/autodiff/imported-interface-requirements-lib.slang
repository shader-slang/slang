//TEST_IGNORE_FILE:

// Library half of `imported-interface-requirements.slang`.
//
// A separate module, so the main module references an interface that belongs to
// a *different* module. Its requirement key carries a mangled name, unlike the
// built-in keys `IDifferentiable` uses, so the two halves of the test exercise
// both kinds of requirement on an interface the main module does not own.

module "imported-interface-requirements-lib";

public interface IScaled
{
    [Differentiable]
    float scaled(float x);
}

public struct Doubler : IScaled
{
    public __init() {}

    [Differentiable]
    public float scaled(float x) { return x * 2.0; }
}
