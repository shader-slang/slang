// Accesses through a `T*` on the GLSL target go through the `_data` member of the
// `buffer_reference` block that represents the pointer, not the handle itself
// (shader-slang/slang#13393).

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=VIA_GLSL): -target spirv -emit-spirv-via-glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV): -target spirv -emit-spirv-directly -entry computeMain -stage compute

struct T
{
    float x;
}

struct Inner
{
    float x;
}

struct S
{
    Inner inner;
    float v[4];
}

cbuffer C
{
    float* pf;
    T* pt;
    S* ps;
    float4* pv;
    int* pi;
}

RWStructuredBuffer<float*> ptrs;
RWStructuredBuffer<float> outF;
RWStructuredBuffer<int> outI;

// VIA_GLSL: OpEntryPoint
// SPV: OpEntryPoint

[numthreads(1, 1, 1)]
void computeMain()
{
    // GLSL: C_0.pf_0._data = 1.0;
    *pf = 1.0;

    T t;
    t.x = 2.0;
    // GLSL: C_0.pt_0._data = t_0;
    *pt = t;

    // GLSL: (C_0.pf_0 + 1)._data = 3.0;
    pf[1] = 3.0;

    // GLSL: ptrs_0._data[uint(0)]._data = 4.0;
    *ptrs[0] = 4.0;

    // GLSL: C_0.ps_0._data.inner_0.x_{{[0-9]+}} = 5.0;
    ps->inner.x = 5.0;

    // GLSL: = C_0.ps_0._data.v_0[1];
    outF[0] = ps->v[1];

    // GLSL: = C_0.pv_0._data[2];
    outF[1] = (*pv)[2];

    // GLSL: C_0.pv_0._data[1] = 6.0;
    (*pv).y = 6.0;

    // GLSL: C_0.pv_0._data.xy = vec2(7.0, 8.0);
    (*pv).xy = float2(7.0, 8.0);

    int old;
    // GLSL: atomicAdd(C_0.pi_0._data, 1)
    InterlockedAdd(*pi, 1, old);
    outI[0] = old;

    // GLSL: atomicExchange((C_0.pi_0 + 1)._data, 5)
    InterlockedExchange(pi[1], 5, old);
    outI[1] = old;

    // GLSL: (C_0.pf_0 + 2)._data = 9.0;
    *(pf + 2) = 9.0;
}
