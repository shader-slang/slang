// A callable shader can forward its incoming empty payload to another callable shader. The
// entry point and outgoing CallShader independently need physical ABI objects. Both use the
// canonical one-uint callable payload type; the logical EmptyData type remains unchanged.

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
// HLSL-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// HLSL-NEXT: };
// HLSL-NOT: uint _slang_dummy
// HLSL: void callableMain(inout {{[A-Za-z0-9_]+}} {{[A-Za-z0-9_]+}})
// HLSL: CallShader(1U, {{[A-Za-z0-9_]+}});
// HLSL-NOT: uint _slang_dummy
// DXIL: define void @
