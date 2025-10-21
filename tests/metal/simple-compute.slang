//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib
//TEST:REFLECTION(filecheck=REFLECT):-target metal -entry computeMain -stage compute
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=1):out,name=outputBuffer
uniform RWStructuredBuffer<float> outputBuffer;

struct MyBlock
{
    StructuredBuffer<float> b1;
    StructuredBuffer<float> b2;
}
//TEST_INPUT: set block = new MyBlock{ ubuffer(data=[7.0]), ubuffer(data=[0.0]) }
ParameterBlock<MyBlock> block;
//TEST_INPUT: set block2 = new MyBlock{ ubuffer(data=[0.0]), ubuffer(data=[5.0]) }
ParameterBlock<MyBlock> block2;

// METAL: {{\[\[}}kernel{{\]\]}} void computeMain(float device* {{.*}} {{\[\[}}buffer(0){{\]\]}}, MyBlock{{.*}} constant* block{{.*}} {{\[\[}}buffer(1){{\]\]}}, MyBlock{{.*}} constant* block2{{.*}} {{\[\[}}buffer(2){{\]\]}})

// METALLIB: define void @computeMain

// REFLECT: "elementVarLayout": {
// REFLECT: "name": "b1",
// REFLECT: "binding": {"kind": "uniform", "offset": 0, "size": 8, "elementStride": 0}

// REFLECT: "name": "b2",
// REFLECT: "binding": {"kind": "uniform", "offset": 8, "size": 8, "elementStride": 0}

// REFLECT: "binding": {"kind": "metalArgumentBufferElement", "index": 0, "count": 2}

// REFLECT: "name": "outputBuffer",
// REFLECT-NEXT: "binding": {"kind": "constantBuffer", "index": 0{{.*}}}

// REFLECT: "name": "block",
// REFLECT-NEXT: "binding": {"kind": "constantBuffer", "index": 1{{.*}}}

void func(float v)
{
    // BUF: 3.0
    outputBuffer[0] = v;
    // BUF: 3.0
    outputBuffer[1] = outputBuffer.Load(0);
    // BUF: 12.0
    outputBuffer[2] = block.b1[0] + block2.b2[0];
}

[numthreads(1,1,1)]
void computeMain()
{
    func(3.0f);
}
