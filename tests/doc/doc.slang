//DISABLE_TEST:SIMPLE:-entry computeMain -target spirv -stage compute -doc

    /// Let's test indent
    /// 
    /// ```
    /// {
    ///     imIndented();
    /// }   
    /// ```
    ///
RWStructuredBuffer<int> inputBuffer;       

/// A struct with some fields
struct SomeStruct
{
    /// A field
    int aField;
    /// Multi-line 
    /// is a thing
    int anotherField;
    int yetAnother; ///< A field with stuff 
};

/// An enum
enum AnEnum
{
    Value, ///< A value
    /// Another value
    /// With a multi-line comment
    AnotherValue,
};


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;       ///< An output buffer

/// doThing!
int doThing(int a,  ///< a parameter
            int b)  ///< b parameter
{
    while (b >= 0)
    {
        a 
        += 
        a;
    }
    
    return a;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int a = dispatchThreadID.x;
    int b = dispatchThreadID.y;
    int c = dispatchThreadID.z;
    int d = a + b * c;
    int e = d + c / 2;
    
    for (int i = 0; i < b; ++i)
    {
        if (e > 10 && i & 2)
        {
            a += b; b -= c; c += c; d = d + e + a; e = a;
        }
        else
        {
            a = e; b = c + c; d += d + __SyntaxError(); e = doThing(e, dispatchThreadID.x);
            
        }
    }
    
    outputBuffer[dispatchThreadID.x] = a + b + c + d + e;
}
