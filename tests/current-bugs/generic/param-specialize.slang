//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj

/* This test trys to specialize an algorithm through generic parameters. 

Using parameters in this way ensures the algorithm in function form 
will have those values known at compile time, and presumably lead to a specialized version.

Does not work because

.slang(11): error 30019: expected an expression of type 'bool', got 'typeof(a)'
    if (a)
*/

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum class Enum
{
    A, B
};

int doThing<a : bool, b : int, c : Enum>()
{
    if (a)
    {
        return 1;
    }
    return 0;
    // A more complex scenario
    //return a ? b + b + int(c) : b - int(c);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int index = dispatchThreadID.x;
    
    let v = doThing<true, false, Enum::A>();
    
    outputBuffer[dispatchThreadID.x] = v;
}