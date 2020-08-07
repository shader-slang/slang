// nested-num.slang

// Test enums defined nested in a struct work as expected. 

//TEST(compute):COMPARE_COMPUTE:

struct Outer
{
    enum Channel
    {
        Red,
        Green,
        Blue,
        Alpha,
    }
    
    static int someValue = 10;
}

int test(int val)
{
    Outer::Channel channel = (Outer::Channel)val; // Outer::Channel(val);
    Outer::Channel otherChannel = channel; // (Outer::Channel)val;

    typedef Outer::Channel Channel;

    int result = 0;
    if(channel == Channel.Red)          result += 1;
    if(channel != Channel.Green)        result += 16;
    if(otherChannel == Channel.Blue)    result += 16*16;
    if(otherChannel != Channel.Alpha)   result += 16*16*16;

    return result;
}


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // This should fail as hueristic says this is a cast(!)
    //int value = (Outer::someValue) +1;
    // This should fail, as heuristic means it's not a cast(!)
    //Outer::Channel anotherValue = (Outer::Channel::Red) + 1;
    
    // These work because of the white space heursitic...
    int value = (Outer::someValue) + 1;
    Outer::Channel anotherValue = (Outer::Channel) +1;
    
    
    uint tid = dispatchThreadID.x;
    int inVal = tid;
    int outVal = test(inVal) + value * 2 + int(anotherValue) * 4;
    outputBuffer[tid] = outVal;
}
