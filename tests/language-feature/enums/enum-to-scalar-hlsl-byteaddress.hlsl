// Coverage for shader-slang/slang#13075 (requested by the reporter on PR #13113):
// a `uint`-backed unscoped enum passed directly as the address argument of
// `ByteAddressBuffer.Load`. This exercises the enum->scalar conversion at a
// builtin-method call site (not just an assignment), confirming the composite
// fires during overload resolution of `Load`.
//
// `PayloadOffset` is referenced by its bare name, which requires the enum to be
// unscoped (its enumerators injected into scope) — so this needs `-unscoped-enum`
// (or an explicit `[UnscopedEnum]`), the same precondition as the rest of the
// feature. `PayloadOffset` is non-zero so the emitted-value leg pins the result.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage fragment -unscoped-enum
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage fragment -unscoped-enum

enum ByteOffset : uint
{
    PayloadOffset = 12
};

ByteAddressBuffer inputBuffer;

[shader("fragment")]
uint main() : SV_Target
{
    // enum -> uint (the declared tag) at the `Load` argument site.
    return inputBuffer.Load(PayloadOffset);
    //HLSL: Load(int(12))
}

// CHECK: OpEntryPoint
