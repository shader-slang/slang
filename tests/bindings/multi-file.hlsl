//TEST:COMPARE_HLSL:-no-mangle -profile sm_4_0 -entry main -stage vertex Tests/bindings/multi-file-extra.hlsl -entry main -stage fragment

// Here we are going to test that we can correctly generating bindings when we
// are presented with a program spanning multiple input files (and multiple entry points)

// This file provides the vertex shader, while the fragment shader resides in
// the file `multi-file-extra.hlsl`

#ifdef __SLANG__
#define R(X) /**/
#define BEGIN_CBUFFER(NAME) cbuffer NAME
#define END_CBUFFER(NAME, REG) /**/
#define CBUFFER_REF(NAME, FIELD) FIELD
#else
#define R(X) X
#define BEGIN_CBUFFER(NAME) struct SLANG_ParameterGroup_##NAME
#define END_CBUFFER(NAME, REG) ; cbuffer NAME : REG { SLANG_ParameterGroup_##NAME NAME; }
#define CBUFFER_REF(NAME, FIELD) NAME.FIELD

#define sharedC     sharedC_0
#define sharedCA    sharedCA_0
#define sharedCB    sharedCB_0
#define sharedCC    sharedCC_0
#define sharedCD    sharedCD_0

#define vertexC     vertexC_0
#define vertexCA    vertexCA_0
#define vertexCB    vertexCB_0
#define vertexCC    vertexCC_0
#define vertexCD    vertexCD_0

#define fragmentC   fragmentC_0
#define fragmentCA  fragmentCA_0
#define fragmentCB  fragmentCB_0
#define fragmentCC  fragmentCC_0
#define fragmentCD  fragmentCD_0

#define sharedS     sharedS_0
#define sharedT     sharedT_0
#define sharedTV    sharedTV_0
#define sharedTF    sharedTF_0

#define vertexS     vertexS_0
#define vertexT     vertexT_0

#define fragmentS     fragmentS_0
#define fragmentT     fragmentT_0

#endif

float4 use(float  val) { return val; };
float4 use(float2 val) { return float4(val,0.0,0.0); };
float4 use(float3 val) { return float4(val,0.0); };
float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s)
{
    // This is the vertex shader, so we can't do implicit-gradient sampling
    return t.SampleGrad(s, 0.0, 0.0, 0.0);
}

// Start with some parameters that will appear in both shaders
Texture2D sharedT R(: register(t0));
SamplerState sharedS R(: register(s0));

BEGIN_CBUFFER(sharedC)
{
    float3 sharedCA;
    float  sharedCB;
    float3 sharedCC;
    float2 sharedCD;
}
END_CBUFFER(sharedC, register(b0))

// Then some parameters specific to this shader
// (these will get placed before the ones in the `extra` file,
// based on how they get named on the command-line)

Texture2D vertexT R(: register(t1));
SamplerState vertexS R(: register(s1));

BEGIN_CBUFFER(vertexC)
{
    float3 vertexCA;
    float  vertexCB;
    float3 vertexCC;
    float2 vertexCD;
}
END_CBUFFER(vertexC, register(b1))

// And end with some shared parameters again
Texture2D sharedTV R(: register(t2));
Texture2D sharedTF R(: register(t3));


float4 main() : SV_POSITION
{
    // Go ahead and use everything here, just to make sure things got placed correctly
    return use(sharedT, sharedS)
        +  use(CBUFFER_REF(sharedC, sharedCD))
        +  use(vertexT, vertexS)
        +  use(CBUFFER_REF(vertexC, vertexCD))
        +  use(sharedTV, vertexS)
        ;
}