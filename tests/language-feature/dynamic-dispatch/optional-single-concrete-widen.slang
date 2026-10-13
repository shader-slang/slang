// A direct-payload {FooImpl, none} tagged union can still flow into a wider
// {FooImpl, BarImpl, none} tagged union without losing either the payload or the `none` case.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[anyValueSize(4)]
interface IFoo
{
    int getValue();
}

struct FooImpl : IFoo
{
    int value;
    int getValue() { return value; }
}

struct BarImpl : IFoo
{
    int value;
    int getValue() { return value * 10; }
}

Optional<IFoo> makeFooOrNone(bool hasValue)
{
    if (hasValue)
    {
        FooImpl result = {7};
        return {result};
    }
    return none;
}

Optional<IFoo> makeWider(int choice)
{
    if (choice == 2)
    {
        BarImpl result = {9};
        return {result};
    }
    return makeFooOrNone(choice == 1);
}

int readValue(int choice)
{
    let value = makeWider(choice);
    if (value.hasValue)
        return value.value.getValue();
    return -1;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = readValue(0); // CHECK: -1
    outputBuffer[1] = readValue(1); // CHECK-NEXT: 7
    outputBuffer[2] = readValue(2); // CHECK-NEXT: 90
}
