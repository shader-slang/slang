// interface-this-type.slang

// Test that `This` type in interface resolves to the concrete conforming type.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

[anyValueSize(8)]
interface IClonable
{
    This clone(int offset);
    int getValue();
}

struct IntBox : IClonable
{
    int val;
    This clone(int offset) { return IntBox(val + offset); }
    int getValue() { return val; }
}

int testClone<T : IClonable>(T obj, int offset)
{
    let cloned = obj.clone(offset);
    return cloned.getValue();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    IntBox box;
    box.val = 10;

    // clone with offset 5 should produce IntBox(15)
    outputBuffer[0] = testClone(box, 5);
    // CHECK: 15

    // chained clone: clone(3) -> IntBox(13), then clone(7) -> IntBox(20)
    let first = box.clone(3);
    outputBuffer[1] = first.clone(7).getValue();
    // CHECK: 20
}
