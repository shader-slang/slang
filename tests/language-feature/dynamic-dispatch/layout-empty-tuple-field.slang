// Tuple<> (empty tuple, zero-size type) as a field in an interface
// implementation alongside other fields. Tests that AnyValue packing
// handles zero-size struct fields correctly.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IItem
{
    float getValue();
}

struct EmptyTupleItem : IItem
{
    Tuple<> empty;
    float value;

    float getValue() { return value; }
}

struct SingleTupleItem : IItem
{
    Tuple<float> single;

    __init(float f)
    {
        single = makeTuple(f);
    }

    float getValue() { return single._0; }
}

IItem makeItem(int id)
{
    if (id == 0)
    {
        EmptyTupleItem item;
        item.value = 42.0;
        return item;
    }
    else
    {
        return SingleTupleItem(7.5);
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // EmptyTupleItem: value = 42.0
    var a = makeItem(id);
    outputBuffer[0] = a.getValue();
    // CHECK: 42.0

    // SingleTupleItem: single._0 = 7.5
    var b = makeItem(id + 1);
    outputBuffer[1] = b.getValue();
    // CHECK: 7.5

    // Verify dispatch returns correct impl for each path
    outputBuffer[2] = a.getValue() + b.getValue();
    // CHECK: 49.5

    outputBuffer[3] = b.getValue() - a.getValue();
    // CHECK: -34.5
}
