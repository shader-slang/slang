// One logical empty struct is used by both a callable entry point and a CallShader caller.
// Both emitted entry points must use the same one-uint representation, without changing the
// logical EmptyData type. The forwarding test also checks both roles in a single entry point.

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

// The default multi-entry invocation emits the callable and ray-generation outputs in turn.
// Each output must contain a one-field dummy type, independent of its generated name.
// HLSL: struct {{[A-Za-z0-9_]+}}
// HLSL-NEXT: {
// HLSL-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// HLSL-NEXT: };
// HLSL-NOT: uint _slang_dummy
// HLSL: void callableMain(inout {{[A-Za-z0-9_]+}} {{[A-Za-z0-9_]+}})
// HLSL: struct {{[A-Za-z0-9_]+}}
// HLSL-NEXT: {
// HLSL-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// HLSL-NEXT: };
// HLSL-NOT: uint _slang_dummy
// HLSL: CallShader(0U, {{[A-Za-z0-9_]+}});
// HLSL-NOT: uint _slang_dummy
