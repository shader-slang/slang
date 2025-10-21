//TEST:REFLECTION(filecheck=METAL): -target metal
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -output-using-type


//METAL: "name": "o",
//METAL: "binding": {"kind": "constantBuffer", "index": 0},

//METAL: "name": "gParams",
//METAL: "binding": {"kind": "constantBuffer", "index": 1},


//METAL: "dir"
//METAL: "binding": {"kind": "uniform", "offset": 0, "size": 16, "elementStride": 4}
//METAL: "color"
//METAL: "binding": {"kind": "uniform", "offset": 16, "size": 16, "elementStride": 4}

struct Params
{
    float3 dir;
    float4 color;
}

//TEST_INPUT: ubuffer(data=[0.0], stride=4):out,name o
RWStructuredBuffer<float> o;
//TEST_INPUT: set gParams = new Params{{3.0, 4.0, 5.0}, {6.0, 7.0 ,8.0, 9.0}}
ParameterBlock<Params> gParams;

[numthreads(1,1,1)]
void computeMain()
{
    // BUF: 3.0
    o[0] = gParams.dir.x;
}
