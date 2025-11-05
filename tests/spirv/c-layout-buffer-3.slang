//TEST:SIMPLE(filecheck=SPIRV): -stage fragment -entry fragmentMain -target spirv -fvk-use-c-layout
struct A {
    int64_t i;
    float f;
};

struct B {
    A a;
    float b;
};

struct FIn
{
    Ptr<B> i0;
    Ptr<B> i1[2][3];
    float c;
    float d;
}

struct FOut
{
	float4 outputColor : SV_Target0;
}

StructuredBuffer<Ptr<B>, Std430DataLayout> testBuffer;

[shader("fragment")]
FOut fragmentMain(FIn input)
{

	FOut output;
	output.outputColor = float4(input.i0.a.f, input.i0.b, input.i1[0][1].a.f, input.i1[0][1].b)+input.c-input.d;
    output.outputColor += testBuffer[0].b;
	return output;
}

// SPIRV: OpDecorate %input_i0 Location 0
// SPIRV: OpDecorate %input_i0 Flat
// SPIRV: OpDecorate %input_i1 Location 1
// SPIRV: OpDecorate %input_i1 Flat
// SPIRV: OpDecorate %input_c Location 7
// SPIRV: OpDecorate %input_d Location 8
// SPIRV: OpMemberDecorate %A_c 0 Offset 0
// SPIRV: OpMemberDecorate %A_c 1 Offset 8
// SPIRV: OpMemberDecorate %B_c 0 Offset 0
// SPIRV: OpMemberDecorate %B_c 1 Offset 16
