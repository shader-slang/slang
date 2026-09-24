// A callable shader can forward its incoming empty payload to another callable shader. Its
// entry-point parameter and the outgoing CallShader argument must use one carrier type with
// exactly one dummy field, while the logical EmptyData type itself remains unchanged.

//TEST:SIMPLE(filecheck=HLSL): -entry callableMain -stage callable -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -entry callableMain -stage callable -target dxil -profile sm_6_7 -validate-ir

struct EmptyData
{
}

[shader("callable")]
void callableMain(inout EmptyData data)
{
    CallShader(1, data);
}

// HLSL: struct {{[A-Za-z0-9_]+}}
// HLSL-NEXT: {
// HLSL-NEXT: int _slang_dummy{{[A-Za-z0-9_]*}};
// HLSL-NEXT: };
// HLSL-NOT: int _slang_dummy
// HLSL: void callableMain(inout {{[A-Za-z0-9_]+}} {{[A-Za-z0-9_]+}})
// HLSL: CallShader(1U, {{[A-Za-z0-9_]+}});
// HLSL-NOT: int _slang_dummy
// DXIL: define void @
