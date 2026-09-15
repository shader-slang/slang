// Regression test for shader-slang/slang#13075: implicit conversion from an
// unscoped `enum` to a builtin scalar type, scoped to the HLSL-flavored dialect.
//
// The conversion is enabled only when both hold: the enum carries
// `UnscopedEnumAttribute` (attached by `-unscoped-enum` or an explicit
// `[UnscopedEnum]`), and the translation unit is `SourceLanguage::HLSL`
// (this is a `.hlsl` file). When enabled, `enum` -> builtin-scalar runs as two
// implicit rounds: enum -> tag type, then tag -> destination.
//
// Enumerators are referenced with the qualified `Color.Green` form so the only
// variable across the directives is the conversion gate, not name lookup.

// Positive: HLSL dialect + `-unscoped-enum` -> the enum is unscoped, so the
// gated enum->scalar conversions compile (including a non-`float` scalar).
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -unscoped-enum

// Negative 1: HLSL dialect but no `-unscoped-enum` -> the plain enum stays
// scoped (no `UnscopedEnumAttribute`), so the conversion is rejected.
//DIAGNOSTIC_TEST:SIMPLE(diag=NOATTR,non-exhaustive): -target spirv -entry computeMain -stage compute

// Negative 2: HLSL dialect + `-unscoped-enum`, but an `enum class` is always
// scoped and never receives `UnscopedEnumAttribute`, so it is rejected too.
//DIAGNOSTIC_TEST:SIMPLE(diag=SCOPED,non-exhaustive): -target spirv -entry computeMain -stage compute -unscoped-enum -DUSE_ENUM_CLASS

#ifdef USE_ENUM_CLASS
enum class Color { Red, Green, Blue };
#else
enum Color { Red, Green, Blue };
#endif

RWStructuredBuffer<float> outF;
RWStructuredBuffer<uint> outU;

[numthreads(1, 1, 1)]
void computeMain()
{
    // enum -> float (the reported case): gated composite enum -> int -> float.
    float f = Color.Green;
    //NOATTR: E30019
    //SCOPED: E30019

    // enum -> uint: a non-`float` builtin scalar, exercising enum -> int -> uint.
    uint u = Color.Green;

    // The reporter's original shape: an enum feeding a `float4` constructor,
    // which relies on the same per-argument enum -> float conversion.
    float4 v = float4(Color.Red, Color.Green, Color.Blue, Color.Green);

    // enum -> bool is unaffected by this feature: `bool` already has a dedicated
    // implicit conversion from any `__EnumType` (core.meta.slang), so it compiles
    // in every configuration and is deliberately excluded from the new composite.
    bool b = Color.Green;

    outF[0] = f + v.x + v.w;
    outU[0] = u + (b ? 1u : 0u);
}

// CHECK: OpEntryPoint
