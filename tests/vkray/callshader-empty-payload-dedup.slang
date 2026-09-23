// Regression test for reusing empty-callable-data carriers: one logical empty struct used
// BOTH as a callable entry point's payload and as a `CallShader` caller's payload, in a single
// module, must use a shared carrier with exactly one dummy field. This checks the emitted
// representation without relying on the collector's implementation or the carrier's type name.
// The source EmptyData type itself must remain unchanged.

//TEST:SIMPLE(filecheck=HLSL): -entry callableMain -stage callable -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=DXIL): -entry callableMain -stage callable -target dxil -profile sm_6_7

struct EmptyData
{
}

[shader("callable")]
void callableMain(inout EmptyData data)
{
    CallShader(1, data);
}

// The receiver and its outgoing call share one carrier containing exactly one dummy field.
// HLSL: struct {{[A-Za-z0-9_]+}}
// HLSL-NEXT: {
// HLSL-NEXT: int _slang_dummy{{[A-Za-z0-9_]*}};
// HLSL-NEXT: };
// HLSL-NOT: int _slang_dummy
// HLSL: void callableMain(inout {{[A-Za-z0-9_]+}} {{[A-Za-z0-9_]+}})
// HLSL: CallShader(1U, {{[A-Za-z0-9_]+}});
// HLSL-NOT: int _slang_dummy
// DXIL: define void @
