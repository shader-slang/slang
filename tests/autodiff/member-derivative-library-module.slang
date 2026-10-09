// Library half of member-derivative-precompiled-module.slang, which precompiles it to a
// .slang-module, so that its methods are deserialized there.
module member_derivative_library_module;

public interface ICurve
{
    [Differentiable]
    public float eval(float t);
}

public struct Cubic : ICurve
{
    [Differentiable]
    [NoDiffThis]
    public float eval(float t) { return t * t * t; }
}
