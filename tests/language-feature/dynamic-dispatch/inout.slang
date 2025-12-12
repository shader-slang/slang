//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Test data-flow through inout parameters (which will require data-flow through 'IRVar' insts)

interface IReserver
{
    [mutating] uint reserve(uint x);
}

struct Single : IReserver
{
    uint total;
    [mutating] uint reserve(uint x) 
    { 
        if (total < x) 
        {
            total = 0;
            return total;
        }
        else
        { 
            total -= x; 
            return x; 
        }
    }
};

struct Double : IReserver
{
    uint total;
    [mutating] uint reserve(uint x) 
    { 
        if (total < 2 * x) 
        {
            total = 0;
            return total;
        }
        else
        { 
            total -= 2 * x; 
            return 2 * x; 
        }
    }
};

IReserver make(uint id, uint total)
{
    if (id == 0)
        return Single(total);
    else
        return Double(total);
}

float f(inout IReserver obj, uint res, float x)
{   
    uint y = obj.reserve(res + 1);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:

    return x * y;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var obj = make(0, 10);
    outputBuffer[0] = f(obj, 0, 2); // CHECK: 2
    outputBuffer[1] = f(obj, 1, 3); // CHECK: 6
    outputBuffer[2] = f(obj, 0, 4); // CHECK: 4

    obj = make(1, 10);
    outputBuffer[3] = f(obj, 0, 2); // CHECK: 4
    outputBuffer[4] = f(obj, 1, 3); // CHECK: 12
    outputBuffer[5] = f(obj, 0, 4); // CHECK: 8
}