//TEST:REFLECTION(filecheck=CHECK): -target metal
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -slang -compute -output-using-type



// CHECK: "name": "gParams",
// CHECK: "binding": {"kind": "constantBuffer", "index": 0},

// CHECK:"name": "pdata",
// CHECK:"binding": {"kind": "uniform", "offset": 0, "size": 16, "elementStride": 4}
// CHECK:"name": "tex",

// Since we will apply MetalArgumentBufferTier2, 'tex' here will just be a uniform.
// The pdata is a nested parameter block, so it will be a 64-bit device pointer which take
// 8 bytes. So the offset of `tex` will be 8 bytes.
// CHECK:"binding": {"kind": "uniform", "offset": 8, "size": 8, "elementStride": 0}


// Check that there will be only two bindings.
//
// CHECK: "name": "output",
// CHECK: "binding": {"kind": "constantBuffer", "index": 1},
//

//TEST_INPUT: set gParams = new Params{new Data{{0,0,0}}, Texture2D(size=4, content=one)}
struct Data { int3 content; }
struct Params
{
    ParameterBlock<Data> pdata;
    Texture2D tex;
}
ParameterBlock<Params> gParams;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<float4> output;

[numthreads(1,1,1)]
void computeMain()
{
    // BUF: 1.0
    // BUF: 1.0
    // BUF: 1.0
    // BUF: 1.0
    output[0] = gParams.tex.Load(gParams.pdata.content);
}
