//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute,vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -output-using-type -xslang -matrix-layout-column-major

// An unsized view must preserve its source buffer's physical matrix layout. Using asymmetric
// input and comparing both off-diagonal elements catches an accidental transpose on conversion.
struct Element
{
    float2x2 matrix;
}

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=16):name=source
StructuredBuffer<Element> source;
//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 0.0], stride=16):name=destination
RWStructuredBuffer<Element> destination;
//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[noinline]
__UnsizedStructuredBuffer<T> makeView<T>(StructuredBuffer<T> buffer)
{
    return buffer;
}

[noinline]
__UnsizedRWStructuredBuffer<T> makeWritableView<T>(RWStructuredBuffer<T> buffer)
{
    return buffer;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    let view = makeView(source);
    output[0] = source[0].matrix[0][1] == view[0].matrix[0][1];
    output[1] = source[0].matrix[1][0] == view[0].matrix[1][0];

    let writableView = makeWritableView(destination);
    writableView[0].matrix[0][1] = 7.0;
    output[2] = destination[0].matrix[0][1] == 7.0;
}

// CHECK: 1
// CHECK: 1
// CHECK: 1
