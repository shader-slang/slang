//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Foo
{
    uint id;
}

int testLessThan(Optional<Foo> opt, uint threshold)
{
    if (let foo = opt)
    {
        if (foo.id < threshold)
            return 1;
        else
            return 2;
    }
    return 0;
}

int testLessThanEqual(Optional<Foo> opt, uint threshold)
{
    if (let foo = opt)
    {
        if (foo.id <= threshold)
            return 1;
        else
            return 2;
    }
    return 0;
}

int testGreaterThan(Optional<Foo> opt, uint threshold)
{
    if (let foo = opt)
    {
        if (foo.id > threshold)
            return 1;
        else
            return 2;
    }
    return 0;
}

int testParenthesizedLessThan(Optional<Foo> opt, uint threshold)
{
    if (let foo = opt)
    {
        if ((foo.id) < threshold)
            return 1;
        else
            return 2;
    }
    return 0;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    Optional<Foo> opt;
    opt = Foo(5);

    int result = 0;
    switch (dispatchThreadID.x)
    {
    case 0:
        // foo.id (5) < 10, should return 1
        // CHECK: 1
        result = testLessThan(opt, 10);
        break;
    case 1:
        // foo.id (5) < 3, should return 2 (not less)
        // CHECK: 2
        result = testLessThan(opt, 3);
        break;
    case 2:
        // foo.id (5) <= 5, should return 1
        // CHECK: 1
        result = testLessThanEqual(opt, 5);
        break;
    case 3:
        // foo.id (5) > 3, should return 1
        // CHECK: 1
        result = testGreaterThan(opt, 3);
        break;
    }

    outputBuffer[dispatchThreadID.x] = result;
}
