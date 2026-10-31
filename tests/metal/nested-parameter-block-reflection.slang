//TEST:REFLECTION(filecheck=METAL): -target metal
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -slang -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -slang -compute -output-using-type


// METAL: "name": "gParams",
// METAL: "binding": {"kind": "constantBuffer", "index": 0},

// METAL:"name": "pdata",
// METAL:"bindings": [
// METAL:{"kind": "uniform", "offset": 0, "size": 8, "elementStride": 0}
// METAL:{"kind": "metalArgumentBufferElement", "index": 0}
// METAL:"name": "tex",

// pdata is a nested parameter block, laid out as a device pointer of 8 bytes and one argument
// buffer slot, so tex lands at byte offset 8 and slot 1.
// METAL:"bindings": [
// METAL:{"kind": "uniform", "offset": 8, "size": 8, "elementStride": 0}
// METAL:{"kind": "metalArgumentBufferElement", "index": 1}


// Check that there will be only two bindings.
//
// METAL: "name": "output",
// METAL: "binding": {"kind": "constantBuffer", "index": 1},
//

//TEST_INPUT: set gParams = new Params{new Data{{0,0,0}}, Texture2D(size=4, content=one)}
struct Data { int3 content; }
struct Params
{
    ParameterBlock<Data> pdata;
    Texture2D tex;
}
ParameterBlock<Params> gParams;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<float4> output;

[numthreads(1,1,1)]
void computeMain()
{
    // BUF-COUNT-4: 1.000000
    output[0] = gParams.tex.Load(gParams.pdata.content);
}
