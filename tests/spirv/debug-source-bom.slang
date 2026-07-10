//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -emit-spirv-directly -entry main -stage compute -preserve-embedded-source
// Verifies that a UTF-8 BOM is stripped from the embedded DebugSource text when a #line directive
// references a file that has no loaded SourceFile (so the DebugSource is read fresh from disk).
// The referenced file (debug-source-bom-original.slang) starts with a UTF-8 BOM (EF BB BF); the
// canonical source-load path decodes/strips it, and the fresh-read fallback used here must match,
// so the embedded OpString for that file must begin with its first source token, not U+FEFF.

// The referenced file's source text begins with "RWStructuredBuffer". If the BOM survived, three
// bytes (EF BB BF) would sit between the opening quote and that token, so the content OpString
// would not start with "RWStructuredBuffer" and this capture would fail to match.
//CHECK-DAG: [[ORIGINAL_TEXT:%[0-9]+]] = OpString "RWStructuredBuffer<float> outputBuffer;
//CHECK-DAG: [[ORIGINAL_FILE:%[0-9]+]] = OpString "tests/spirv/debug-source-bom-original.slang"
// Bind the DebugSource for the #line file to the BOM-free content OpString captured above, so the
// test proves the fresh-read fallback embeds decoded (BOM-stripped) text for exactly that file.
//CHECK-DAG: {{.*}} DebugSource [[ORIGINAL_FILE]] [[ORIGINAL_TEXT]]

#line 1 "tests/spirv/debug-source-bom-original.slang"
RWStructuredBuffer<float> outputBuffer;

float helper(float value)
{
    return value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = helper(1.0);
}
