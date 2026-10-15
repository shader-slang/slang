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
// variable across the directives is the conversion gate, not name lookup. Their
// values are non-zero and distinct so the emitted-value leg below distinguishes
// a correct conversion from a zero-init or a wrong ordinal.

// Positive (compile, GPU target): HLSL dialect + `-unscoped-enum` -> the enum is
// unscoped, so the gated enum->scalar conversions compile.
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -unscoped-enum

// Positive (emitted value): the same conversions must produce the correct
// numeric results, not merely compile. Compiling to HLSL uses the direct-compile
// path where the dialect gate is satisfied; each converted enumerator const-folds,
// so the emitted stores pin the values (a wrong ordinal or a truncated tag would
// change them). (COMPARE_COMPUTE is not usable here: its module-load path leaves
// the translation unit's `sourceLanguage` non-HLSL, so the gate would not fire.)
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -unscoped-enum

// Negative 1: HLSL dialect but no `-unscoped-enum` -> the plain enum stays
// scoped (no `UnscopedEnumAttribute`), so each implicit enum->scalar is rejected.
//DIAGNOSTIC_TEST:SIMPLE(diag=NOATTR,non-exhaustive): -target spirv -entry computeMain -stage compute

// Negative 2: HLSL dialect + `-unscoped-enum`, but an `enum class` is always
// scoped and never receives `UnscopedEnumAttribute`, so it is rejected too.
//DIAGNOSTIC_TEST:SIMPLE(diag=SCOPED,non-exhaustive): -target spirv -entry computeMain -stage compute -unscoped-enum -DUSE_ENUM_CLASS

#ifdef USE_ENUM_CLASS
enum class Color { Red = 3, Green = 7, Blue = 9 };
#else
enum Color { Red = 3, Green = 7, Blue = 9 };
#endif

RWStructuredBuffer<float> outF;

[numthreads(1, 1, 1)]
void computeMain()
{
    // enum -> float (the reported case): gated composite enum -> tag -> float.
    float f = Color.Green;
    //NOATTR: E30019
    //SCOPED: E30019

    // enum -> uint: a non-`float` builtin scalar, exercising enum -> tag -> uint.
    uint u = Color.Blue;
    //NOATTR: E30019
    //SCOPED: E30019

    // The reporter's original shape: enums feeding a `float4` constructor, which
    // relies on the same per-argument enum -> float conversion.
    float4 v = float4(Color.Red, Color.Green, Color.Blue, Color.Green);
    //NOATTR: E30019
    //SCOPED: E30019

    // enum -> bool is unaffected by this feature: `bool` already has a dedicated
    // implicit conversion from any `__EnumType` (core.meta.slang), so it compiles
    // in every configuration and is deliberately excluded from the new composite.
    bool b = Color.Green;

    outF[0] = f;                     // Color.Green         -> 7
    outF[1] = (float)u;              // Color.Blue          -> 9
    outF[2] = v.x + v.y + v.z + v.w; // 3 + 7 + 9 + 7       -> 26
    outF[3] = b ? 1.0f : 0.0f;       // Color.Green -> true -> 1
    //HLSL: outF{{.*}} = 7.0f;
    //HLSL: outF{{.*}} = 9.0f;
    //HLSL: outF{{.*}} = 26.0f;
    //HLSL: outF{{.*}} = 1.0f;
}

// CHECK: OpEntryPoint
