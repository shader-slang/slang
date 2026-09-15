// Companion positive for shader-slang/slang#13075: in the HLSL dialect, an
// explicit `[UnscopedEnum]` attribute is enough to enable the enum->scalar
// conversion, with no `-unscoped-enum` command-line flag. This exercises the
// `UnscopedEnumAttribute` half of `isUnscopedEnum` on its own.
//
// The enum also has an explicit non-`int` tag type (`uint`), so this confirms
// the composite converts through the *declared* tag (enum -> uint -> float)
// rather than assuming `int`.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

[UnscopedEnum]
enum Color : uint { Red, Green, Blue };

RWStructuredBuffer<float> outF;

[numthreads(1, 1, 1)]
void computeMain()
{
    float f = Color.Green;
    outF[0] = f;
}

// CHECK: OpEntryPoint
