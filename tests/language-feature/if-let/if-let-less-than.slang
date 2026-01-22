//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Foo
{
    uint id;
}

struct Bar
{
    uint value;
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

int testNestedIfLet(Optional<Foo> optFoo, Optional<Bar> optBar)
{
    if (let foo = optFoo)
    {
        if (let bar = optBar)
        {
            // Nested if-let: both foo and bar should be in scope
            if (foo.id < bar.value)
                return 1;
            else if (bar.value < foo.id)
                return 2;
            else
                return 3;
        }
        return 4;
    }
    return 0;
}

int testNestedIfLetChained(Optional<Foo> opt1, Optional<Foo> opt2, uint threshold)
{
    if (let a = opt1)
    {
        if (let b = opt2)
        {
            // Both a.id and b.id with < operator
            if (a.id < threshold && b.id < threshold)
                return 1;
            else
                return 2;
        }
    }
    return 0;
}

[numthreads(6, 1, 1)]
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
    case 4:
        // Nested if-let: foo.id (5) < bar.value (10), should return 1
        // CHECK: 1
        {
            Optional<Bar> optBar;
            optBar = Bar(10);
            result = testNestedIfLet(opt, optBar);
        }
        break;
    case 5:
        // Nested if-let chained: a.id (5) < 10 && b.id (3) < 10, should return 1
        // CHECK: 1
        {
            Optional<Foo> opt2;
            opt2 = Foo(3);
            result = testNestedIfLetChained(opt, opt2, 10);
        }
        break;
    }

    outputBuffer[dispatchThreadID.x] = result;
}
