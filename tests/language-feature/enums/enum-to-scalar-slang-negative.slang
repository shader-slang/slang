// Companion negative for shader-slang/slang#13075: the implicit enum->scalar
// conversion is scoped to the HLSL-flavored dialect, so it must NOT fire for a
// Slang translation unit even when the enum is unscoped (`-unscoped-enum`).
//
// This is a `.slang` file, so its `SourceLanguage` is `Slang`; the enum here
// does carry `UnscopedEnumAttribute` (from `-unscoped-enum`), which isolates
// the `SourceLanguage::HLSL` half of the gate. The reporter's original repro is
// exactly this shape, and it remains `E30019` by design.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry computeMain -stage compute -unscoped-enum

enum Color { Red, Green, Blue }

RWStructuredBuffer<float> outF;

[numthreads(1, 1, 1)]
void computeMain()
{
    float f = Color.Green;
    //CHECK: E30019
    outF[0] = f;
}
