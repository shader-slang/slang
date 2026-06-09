// GH Issue #7412: Capabilities of `extension` should be a subset of the target type's capabilities.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[require(glsl)]
struct Bar
{
}

// ext1: extension declares [require(hlsl)] but Bar only supports glsl -- should error
[require(hlsl)]
extension Bar
//CHECK:  ^^^ error E36100
//CHECK:  ^^^ requires capability
{
}

// ext2: member function in extension declares [require(hlsl)] but Bar only supports glsl -- should error
extension Bar
{
    [require(hlsl)]
    void foo2(){}
//CHECK: ^^^^ error E36100
//CHECK: ^^^^ requires capability
}

// ext3: extension has compatible [require(glsl)], but member function has [require(hlsl)] -- should error
[require(glsl)]
struct Baz
{
}

[require(glsl)]
extension Baz
{
    [require(hlsl)]
    void foo3(){}
//CHECK: ^^^^ error E36100
//CHECK: ^^^^ requires capability
}

// ext4: constructor in extension declares [require(hlsl)] but Bar only supports glsl -- should error
extension Bar
{
    [require(hlsl)]
    __init(int x) {}
/*CHECK:
    ^^^^^^ error E36100
    ^^^^^^ requires capability
*/
}

// ext5: subscript in extension declares [require(hlsl)] but Bar only supports glsl -- should error
extension Bar
{
    [require(hlsl)]
    __subscript(int i) -> int
//CHECK:       ^ error E36100
//CHECK:       ^ requires capability
    { get { return 0; } }
}

// ext6: generic extension with extension-level [require(hlsl)] on a [require(glsl)] type -- should error
[require(glsl)]
struct GBox<T> { T x; }

[require(hlsl)]
extension<T> GBox<T>
//CHECK:     ^^^^ error E36100
//CHECK:     ^^^^ requires capability
{
}

// ext7: member of generic extension with [require(hlsl)] on a [require(glsl)] type -- should error
extension<T> GBox<T>
{
    [require(hlsl)]
    void gfoo(){}
//CHECK: ^^^^ error E36100
//CHECK: ^^^^ requires capability
}

// ext8: stage capability conflict -- extension requires compute but type only supports fragment
[require(fragment)]
struct FragOnly {}

[require(compute)]
extension FragOnly
//CHECK:  ^^^^^^^^ error E36100
//CHECK:  ^^^^^^^^ requires capability
{
}

// ext9: extension-level [require] fires even when all members are static
// The extension declaration itself has [require(hlsl)], which conflicts with Bar's [require(glsl)].
[require(hlsl)]
extension Bar
//CHECK:  ^^^ error E36100
//CHECK:  ^^^ requires capability
{
    static void staticOnly() {}
}

// ext10: accessor-level [require(hlsl)] inside an extension subscript -- should error
extension Bar
{
    __subscript(int i) -> int
    {
        [require(hlsl)] get { return 0; }
//CHECK:                ^^^ error E36100
//CHECK:                ^^^ requires capability
    }
}

// ext11: set accessor with [require(hlsl)] in a [require(glsl)] extension -- should error
extension Bar
{
    __subscript(int i) -> int
    {
        get { return 0; }
        [require(hlsl)] set {}
//CHECK:                ^^^ error E36100
//CHECK:                ^^^ requires capability
    }
}

// ext12: capability alias on extension that is disjoint from target -- should error
// hlsl_spirv = hlsl | spirv; Bar only supports glsl, so the intersection is empty.
[require(hlsl_spirv)]
extension Bar
//CHECK:  ^^^ error E36100
//CHECK:  ^^^ requires capability
{
}

// ext_combined: extension-level conflict with a non-static member that has no own [require].
// Only the extension-level error must fire. _checkExtensionMemberCapConflict checks own caps
// only (not the joined declaredCaps), so `inner` -- which carries no [require] of its own --
// does not produce a second E36100. A refactor that switches to checking declaredCaps would
// cause `inner` to also fire, which the exhaustive diagnostic check would catch.
[require(hlsl)]
extension Bar
//CHECK:  ^^^ error E36100
//CHECK:  ^^^ requires capability
{
    void inner() {}
}

// ext_both: extension AND member each have their own incompatible [require].
// Both errors must fire independently: the member's own [require(hlsl)] is disjoint from
// Bar's [require(glsl)], so the cascade-suppression logic (own-caps-only) does not apply
// here -- the member is independently checked and must also produce E36100.
[require(hlsl)]
extension Bar
//CHECK:  ^^^ error E36100
//CHECK:  ^^^ requires capability
{
    [require(hlsl)]
    void m() {}
//CHECK: ^ error E36100
//CHECK: ^ requires capability
}

// ext13: property-level [require(hlsl)] in a [require(glsl)] extension -- should error
extension Bar
{
    [require(hlsl)]
    property int prop
//CHECK:         ^^^^ error E36100
//CHECK:         ^^^^ requires capability
    { get { return 0; } }
}
