// GH Issue #7412: Capabilities of `extension` should be a subset of the target type's capabilities.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[require(glsl)]
struct Bar
{
}

// ext1: extension declares [require(hlsl)] but Bar only supports glsl -- should error
[require(hlsl)]
extension Bar
//CHECK:  ^^^ conflicting capability requirement
//CHECK:  ^^^ requires capability
{
}

// ext2: member function in extension declares [require(hlsl)] but Bar only supports glsl -- should error
extension Bar
{
    [require(hlsl)]
    void foo2(){}
//CHECK: ^^^^ conflicting capability requirement
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
//CHECK: ^^^^ conflicting capability requirement
//CHECK: ^^^^ requires capability
}

// ext4: constructor in extension declares [require(hlsl)] but Bar only supports glsl -- should error
extension Bar
{
    [require(hlsl)]
    __init(int x) {}
/*CHECK:
    ^^^^^^ conflicting capability requirement
    ^^^^^^ requires capability
*/
}

// ext5: subscript in extension declares [require(hlsl)] but Bar only supports glsl -- should error
extension Bar
{
    [require(hlsl)]
    __subscript(int i) -> int
//CHECK:       ^ conflicting capability requirement
//CHECK:       ^ requires capability
    { get { return 0; } }
}
