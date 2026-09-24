// Regression test for reusing empty-callable-data carriers: one logical empty struct used
// BOTH as a callable entry point's payload and as a `CallShader` caller's payload, in a single
// module, must use a shared carrier with exactly one dummy field. This checks the emitted
// representation without relying on the collector's implementation or the carrier's type name.
// The source EmptyData type itself must remain unchanged.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile sm_6_7

struct EmptyData
{
}

[shader("callable")]
void callableMain(inout EmptyData data)
{
}

[shader("raygeneration")]
void rgen()
{
    EmptyData d;
    CallShader(0, d);
}

// The callable entry point and the ray-generation call share one carrier containing exactly one
// dummy field.
// HLSL: struct {{[A-Za-z0-9_]+}}
// HLSL-NEXT: {
// HLSL-NEXT: int _slang_dummy{{[A-Za-z0-9_]*}};
// HLSL-NEXT: };
// HLSL-NOT: int _slang_dummy
// HLSL: void callableMain(inout {{[A-Za-z0-9_]+}} {{[A-Za-z0-9_]+}})
// HLSL: CallShader(0U, {{[A-Za-z0-9_]+}});
// HLSL-NOT: int _slang_dummy
