implementing raytracing;

namespace rt
{

// These are compiler-provided, zero-storage views. Properties are added only with their matching
// IR operations and lowering support.
[require(closesthit)]
public struct ClosestHitInput<Context>
    where Context : IHitContext
{}

[require(anyhit)]
public struct AnyHitInput<Context>
    where Context : IHitContext
{}

[require(intersection)]
public struct IntersectionInput<Context>
    where Context : IHitContext
{}

[require(miss)]
public struct MissInput<Context>
    where Context : IMissGroupContext
{}

[require(callable)]
public struct CallableInput<Context>
    where Context : ICallableGroupContext
{}

} // namespace rt
