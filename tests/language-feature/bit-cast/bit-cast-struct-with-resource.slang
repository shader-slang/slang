// Test that bit_cast between aggregates containing resource fields copies each
// resource field and reinterprets the remaining ordinary data (issue #13480).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -llvm -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute

// SPIRV: OpEntryPoint
// HLSL: computeMain

struct View
{
    StructuredBuffer<float> data;
    uint offset;
}

struct ViewAlias
{
    StructuredBuffer<float> data;
    uint offset;
}

// The ordinary fields differ in type but match in size and alignment.
struct Tensor
{
    StructuredBuffer<float> data;
    uint[3] shape;
    float scale;
}

struct PrimalTensor
{
    StructuredBuffer<float> data;
    uint[3] shape;
    uint scaleBits;
}

struct Outer
{
    View view;
    int bias;
}

struct OuterAlias
{
    ViewAlias view;
    uint bias;
}

// An explicit offset that matches the natural one does not change the layout.
struct ExplicitView
{
    [[vk::offset(0)]]
    StructuredBuffer<float> data;
    uint offset;
}

//TEST_INPUT:set gData = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4)
StructuredBuffer<float> gData;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    View v = { gData, 2 };
    ViewAlias va = bit_cast<ViewAlias, View>(v);
    // CHECK: type: uint32_t
    // CHECK-NEXT: 3
    outputBuffer[0] = uint(va.data[va.offset]);

    Tensor t = { gData, { 4, 5, 6 }, 1.5 };
    PrimalTensor p = bit_cast<PrimalTensor, Tensor>(t);
    // CHECK-NEXT: 6
    outputBuffer[1] = p.shape[2];
    // CHECK-NEXT: 1069547520
    outputBuffer[2] = p.scaleBits;
    // CHECK-NEXT: 5
    outputBuffer[3] = uint(p.data[p.shape[0]]);

    Outer o = { { gData, 1 }, -1 };
    OuterAlias oa = bit_cast<OuterAlias, Outer>(o);
    // CHECK-NEXT: 2
    outputBuffer[4] = uint(oa.view.data[oa.view.offset]);
    // CHECK-NEXT: 4294967295
    outputBuffer[5] = oa.bias;

    View[2] arr = { { gData, 6 }, { gData, 7 } };
    ViewAlias[2] arrAlias = bit_cast<ViewAlias[2], View[2]>(arr);
    // CHECK-NEXT: 7
    outputBuffer[6] = uint(arrAlias[0].data[arrAlias[0].offset]);
    // CHECK-NEXT: 8
    outputBuffer[7] = uint(arrAlias[1].data[arrAlias[1].offset]);

    ExplicitView ev = bit_cast<ExplicitView, View>({ gData, 3 });
    // CHECK-NEXT: 4
    outputBuffer[8] = uint(ev.data[ev.offset]);
}
