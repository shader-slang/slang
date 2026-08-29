//DISABLED_TEST(compute):COMPARE_COMPUTE: -shaderobj
//TEST(compute):COMPARE_COMPUTE: -shaderobj -cuda
//TEST(compute):COMPARE_COMPUTE: -shaderobj -cuda -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -specialize Material<1,2> -stage compute -capability cuda_sm_7_0 -O3 -o -

// NVVM: .visible .entry computeMain()
// NVVM: mov.u32 {{%r[0-9]+}}, 3;
// NVVM: st.global.u32

//TEST_INPUT:type Material<1,2>

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IBRDF
{
    int compute();
};

interface IMaterial
{
    associatedtype TBRDF : IBRDF;
    TBRDF getBRDF();
}

struct BRDF<let A:int, let B:int> : IBRDF
{
    int c;
    int compute()
    {
        return A+B;        
    }
};

struct Material<let A:int, let B: int> : IMaterial
{
    typedef BRDF<A,B> TBRDF;
    TBRDF getBRDF() { TBRDF a; a.c = 0; return a; }
};

[numthreads(1, 1, 1)]
void computeMain<M : IMaterial>(
    uniform M material,
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
    M.TBRDF brdf = material.getBRDF();
	int outVal = brdf.compute();
	outputBuffer[dispatchThreadID.x] = outVal;
}
