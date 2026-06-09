// GH Issue #7412: Valid extension capability patterns that must NOT produce errors.
// This is the negative counterpart to gh-7412.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Case 1: Extension with no [require] on a type with [require(glsl)].
// Neither the extension nor its members add any cap constraint.
[require(glsl)]
struct Foo
{
}

extension Foo
{
    void noReq() {}
}

// Case 2: Extension with matching [require(glsl)] on a [require(glsl)] type.
[require(glsl)]
extension Foo
{
}

// Case 3: Member function with the same capability as the type.
extension Foo
{
    [require(glsl)]
    void sameReq() {}
}

// Case 4: Type with a union of targets; member declares a valid subset.
[require(glsl)]
[require(hlsl)]
struct Multi
{
}

extension Multi
{
    [require(hlsl)]
    void onlyHlsl() {}

    [require(glsl)]
    void onlyGlsl() {}
}

// Case 5: Type with no capability constraint; any extension caps are fine.
struct Plain
{
}

extension Plain
{
    [require(hlsl)]
    void anyTarget() {}
}

// Case 6: Generic extension; target type has no explicit caps.
struct Box<T>
{
}

extension<T> Box<T>
{
    [require(hlsl)]
    void doThing() {}
}

// Case 7: Static extension member with incompatible [require] on the target type.
// Static members do not carry an implicit 'this' constraint, so the subset check
// is intentionally not applied to them (consistent with how static struct methods
// are handled). No error expected.
[require(glsl)]
struct StaticHost
{
}

extension StaticHost
{
    [require(hlsl)]
    static void staticHelper() {}
}

// Case 8: Constructor with compatible [require] in extension -- no error.
extension Foo
{
    [require(glsl)]
    __init(int x) {}
}

// Case 9: Subscript with compatible [require] in extension -- no error.
extension Foo
{
    [require(glsl)]
    __subscript(int i) -> int { get { return 0; } }
}

// Case 10: Generic extension with compatible caps on a typed target.
[require(glsl)]
struct GFoo<T>
{
}

extension<T> GFoo<T>
{
    [require(glsl)]
    void compatThing() {}
}

// Case 11: Stage cap -- extension member with matching stage requirement.
[require(compute)]
struct ComputeOnly {}

extension ComputeOnly
{
    [require(compute)]
    void computeMethod() {}
}

// Case 12: Accessor-level [require] compatible with the target type -- no error.
extension Foo
{
    __subscript(int i) -> int
    {
        [require(glsl)] get { return 0; }
    }
}

// Case 13: set accessor with compatible [require] -- no error.
extension Foo
{
    __subscript(int i) -> int
    {
        get { return 0; }
        [require(glsl)] set {}
    }
}

// Case 14: Capability alias whose expansion intersects the target's caps -- no error.
// glsl_spirv = glsl | spirv; Foo is [require(glsl)], so intersection = {glsl} is non-empty.
[require(glsl_spirv)]
extension Foo
{
}

// Case 15: Stacked [require] on the extension where the union intersects target's caps.
// Extension declares both glsl and hlsl support; Foo only supports glsl.
// Under non-empty-intersection semantics this must NOT error (intersection = {glsl}).
[require(glsl)]
[require(hlsl)]
extension Foo
{
}

// Case 16: Write-back invariant: member [require] that is valid vs the target type but
// would be incorrectly flagged if visitExtensionDecl wrote the intersected caps back to
// extensionDecl->inferredCapabilityRequirements.
// M supports both glsl and hlsl. The extension only requires glsl, so the check passes
// (intersection = {glsl}). The member requires hlsl, which is also in M's caps, so the
// member-level check passes too. If visitExtensionDecl wrote back {glsl} (the intersection)
// to the extension, member-level or parent-chain logic could see the wrong extension caps
// and produce a false positive.
[require(glsl)]
[require(hlsl)]
struct M {}

[require(glsl)]
extension M
{
    [require(hlsl)]
    void onlyHlsl() {}
}

// Case 17: Property with compatible [require] in extension -- no error.
extension Foo
{
    [require(glsl)]
    property int prop { get { return 0; } }
}

// Case 18: Extension of a type-parameter (extension<T:IFoo> T).
// The target is a GenericTypeParamDecl, whose inferredCapabilityRequirements is null.
// _propagateRequirement tolerates null nodeCaps and no-ops — no error expected.
// This is intentional: extensions on unconstrained/interface-constrained type params
// are not currently checked against the param's (unknown) capabilities.
interface IHasMethod
{
    void method();
}

extension<T : IHasMethod> T
{
    [require(glsl)]
    void extraGlsl() {}
}
