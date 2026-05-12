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
