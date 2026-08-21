implementing raytracing;

namespace rt
{

// These are compiler-provided, zero-storage views. Properties are added only with their matching
// IR operations and lowering support.
public struct ClosestHitInput<Context>
    where Context : IHitContext
{}

public struct AnyHitInput<Context>
    where Context : IHitContext
{}

public struct IntersectionInput<Context>
    where Context : IHitContext
{}

public struct MissInput<Context>
    where Context : IMissGroupContext
{}

public struct CallableInput<Context>
    where Context : ICallableGroupContext
{}

} // namespace rt
