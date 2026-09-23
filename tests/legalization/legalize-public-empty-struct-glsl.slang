// Regression for shader-slang/slang#11917: a public empty struct must not survive to GLSL emit,
// where an empty `struct {}` is illegal.
//
// On the Khronos/HLSL family, `legalizeResourceTypes` runs and strips empty structs (its
// `isSimpleType` is unconditionally false — it does so even for public-interface empties, which the
// later `legalizeEmptyTypes` would keep). This shader has a public empty `struct` used as a
// parameter and no resource type, so it exercises that empty-struct stripping in isolation; the test
// guards that the shared type-legalizer's worklist rework preserves it.
//
// Emitting through glslang (`-emit-spirv-via-glsl`) is the guard: glslang rejects an empty
// `struct {}` with a syntax error, so this compiles at all only if the empty struct was stripped
// before emit. The command must succeed before FileCheck runs, and the `Glslang` generator line
// confirms the SPIR-V came from the requested glslang path.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -emit-spirv-via-glsl -stage fragment -entry main

public struct Empty
{
};

public float4 useEmpty(Empty e, float4 c)
{
    return c;
}

float4 main(Empty e) : SV_Target
{
    return useEmpty(e, float4(1, 0, 0, 1));
}

// CHECK: Glslang
