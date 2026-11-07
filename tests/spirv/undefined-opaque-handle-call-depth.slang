// Check that a never-written texture passed down a long chain of calls, or down both branches of a
// call tree, compiles to SPIR-V without an `OpUndef` of the image type, and that each function on
// the path gets one specialized clone, so that the code does not grow with the number of paths.

//TEST:SIMPLE(filecheck=CHAIN):-target spirv -entry passDownLongChain -stage compute
//TEST:SIMPLE(filecheck=TREE):-target spirv -entry passDownCallTree -stage compute -O0

// CHAIN: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// CHAIN-NOT: OpUndef %[[IMG]]

// TREE: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// TREE-NOT: OpUndef %[[IMG]]

// The call tree has seven functions, `tree0` to `tree6`. The calls with `gTexture` use them as
// they are, and the calls with `t` use one clone of each, so the module holds fourteen of them
// besides the entry point.
// TREE: %passDownCallTree = OpFunction
// TREE-COUNT-14: = OpFunction %v4float None
// TREE-NOT: = OpFunction {{.*}} None

Texture2D<float4> gTexture;
SamplerState gSampler;
RWStructuredBuffer<float4> outputBuffer;
cbuffer Settings
{
    bool useUndefined;
};

float chain70(Texture2D<float4> t)
{
    return t.SampleLevel(gSampler, float2(0), 0).x;
}
float chain69(Texture2D<float4> t) { return chain70(t) * 2.0; }
float chain68(Texture2D<float4> t) { return chain69(t) * 2.0; }
float chain67(Texture2D<float4> t) { return chain68(t) * 2.0; }
float chain66(Texture2D<float4> t) { return chain67(t) * 2.0; }
float chain65(Texture2D<float4> t) { return chain66(t) * 2.0; }
float chain64(Texture2D<float4> t) { return chain65(t) * 2.0; }
float chain63(Texture2D<float4> t) { return chain64(t) * 2.0; }
float chain62(Texture2D<float4> t) { return chain63(t) * 2.0; }
float chain61(Texture2D<float4> t) { return chain62(t) * 2.0; }
float chain60(Texture2D<float4> t) { return chain61(t) * 2.0; }
float chain59(Texture2D<float4> t) { return chain60(t) * 2.0; }
float chain58(Texture2D<float4> t) { return chain59(t) * 2.0; }
float chain57(Texture2D<float4> t) { return chain58(t) * 2.0; }
float chain56(Texture2D<float4> t) { return chain57(t) * 2.0; }
float chain55(Texture2D<float4> t) { return chain56(t) * 2.0; }
float chain54(Texture2D<float4> t) { return chain55(t) * 2.0; }
float chain53(Texture2D<float4> t) { return chain54(t) * 2.0; }
float chain52(Texture2D<float4> t) { return chain53(t) * 2.0; }
float chain51(Texture2D<float4> t) { return chain52(t) * 2.0; }
float chain50(Texture2D<float4> t) { return chain51(t) * 2.0; }
float chain49(Texture2D<float4> t) { return chain50(t) * 2.0; }
float chain48(Texture2D<float4> t) { return chain49(t) * 2.0; }
float chain47(Texture2D<float4> t) { return chain48(t) * 2.0; }
float chain46(Texture2D<float4> t) { return chain47(t) * 2.0; }
float chain45(Texture2D<float4> t) { return chain46(t) * 2.0; }
float chain44(Texture2D<float4> t) { return chain45(t) * 2.0; }
float chain43(Texture2D<float4> t) { return chain44(t) * 2.0; }
float chain42(Texture2D<float4> t) { return chain43(t) * 2.0; }
float chain41(Texture2D<float4> t) { return chain42(t) * 2.0; }
float chain40(Texture2D<float4> t) { return chain41(t) * 2.0; }
float chain39(Texture2D<float4> t) { return chain40(t) * 2.0; }
float chain38(Texture2D<float4> t) { return chain39(t) * 2.0; }
float chain37(Texture2D<float4> t) { return chain38(t) * 2.0; }
float chain36(Texture2D<float4> t) { return chain37(t) * 2.0; }
float chain35(Texture2D<float4> t) { return chain36(t) * 2.0; }
float chain34(Texture2D<float4> t) { return chain35(t) * 2.0; }
float chain33(Texture2D<float4> t) { return chain34(t) * 2.0; }
float chain32(Texture2D<float4> t) { return chain33(t) * 2.0; }
float chain31(Texture2D<float4> t) { return chain32(t) * 2.0; }
float chain30(Texture2D<float4> t) { return chain31(t) * 2.0; }
float chain29(Texture2D<float4> t) { return chain30(t) * 2.0; }
float chain28(Texture2D<float4> t) { return chain29(t) * 2.0; }
float chain27(Texture2D<float4> t) { return chain28(t) * 2.0; }
float chain26(Texture2D<float4> t) { return chain27(t) * 2.0; }
float chain25(Texture2D<float4> t) { return chain26(t) * 2.0; }
float chain24(Texture2D<float4> t) { return chain25(t) * 2.0; }
float chain23(Texture2D<float4> t) { return chain24(t) * 2.0; }
float chain22(Texture2D<float4> t) { return chain23(t) * 2.0; }
float chain21(Texture2D<float4> t) { return chain22(t) * 2.0; }
float chain20(Texture2D<float4> t) { return chain21(t) * 2.0; }
float chain19(Texture2D<float4> t) { return chain20(t) * 2.0; }
float chain18(Texture2D<float4> t) { return chain19(t) * 2.0; }
float chain17(Texture2D<float4> t) { return chain18(t) * 2.0; }
float chain16(Texture2D<float4> t) { return chain17(t) * 2.0; }
float chain15(Texture2D<float4> t) { return chain16(t) * 2.0; }
float chain14(Texture2D<float4> t) { return chain15(t) * 2.0; }
float chain13(Texture2D<float4> t) { return chain14(t) * 2.0; }
float chain12(Texture2D<float4> t) { return chain13(t) * 2.0; }
float chain11(Texture2D<float4> t) { return chain12(t) * 2.0; }
float chain10(Texture2D<float4> t) { return chain11(t) * 2.0; }
float chain9(Texture2D<float4> t) { return chain10(t) * 2.0; }
float chain8(Texture2D<float4> t) { return chain9(t) * 2.0; }
float chain7(Texture2D<float4> t) { return chain8(t) * 2.0; }
float chain6(Texture2D<float4> t) { return chain7(t) * 2.0; }
float chain5(Texture2D<float4> t) { return chain6(t) * 2.0; }
float chain4(Texture2D<float4> t) { return chain5(t) * 2.0; }
float chain3(Texture2D<float4> t) { return chain4(t) * 2.0; }
float chain2(Texture2D<float4> t) { return chain3(t) * 2.0; }
float chain1(Texture2D<float4> t) { return chain2(t) * 2.0; }
float chain0(Texture2D<float4> t) { return chain1(t) * 2.0; }

[numthreads(1, 1, 1)]
void passDownLongChain()
{
    Texture2D<float4> t;
    float r = chain0(gTexture);
    if (useUndefined)
        r = chain0(t);
    outputBuffer[0] = r;
}

float4 tree6(Texture2D<float4> t, float x)
{
    return t.SampleLevel(gSampler, float2(x), 0);
}

float4 tree5(Texture2D<float4> t, float x)
{
    return tree6(t, x) + tree6(t, x + 1.0);
}

float4 tree4(Texture2D<float4> t, float x)
{
    return tree5(t, x) + tree5(t, x + 1.0);
}

float4 tree3(Texture2D<float4> t, float x)
{
    return tree4(t, x) + tree4(t, x + 1.0);
}

float4 tree2(Texture2D<float4> t, float x)
{
    return tree3(t, x) + tree3(t, x + 1.0);
}

float4 tree1(Texture2D<float4> t, float x)
{
    return tree2(t, x) + tree2(t, x + 1.0);
}

float4 tree0(Texture2D<float4> t, float x)
{
    return tree1(t, x) + tree1(t, x + 1.0);
}

[numthreads(1, 1, 1)]
void passDownCallTree()
{
    Texture2D<float4> t;
    float4 r = tree0(gTexture, 0.0);
    if (useUndefined)
        r = tree0(t, 1.0);
    outputBuffer[0] = r;
}
