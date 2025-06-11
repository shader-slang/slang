//TEST:REFLECTION(filecheck=CHECK): -target metal


//CHECK: "name": "o",
//CHECK: "binding": {"kind": "constantBuffer", "index": 0},

//CHECK: "name": "gParams",
//CHECK: "binding": {"kind": "constantBuffer", "index": 1},


//CHECK: "dir"
//CHECK: "binding": {"kind": "uniform", "offset": 0, "size": 16}
//CHECK: "color"
//CHECK: "binding": {"kind": "uniform", "offset": 16, "size": 16}

struct Params
{
    float3 dir;
    float4 color;
}

RWStructuredBuffer<float> o;
ParameterBlock<Params> gParams;

[numthreads(1,1,1)]
void computeMain()
{
    o[0] = gParams.dir.x;
}
