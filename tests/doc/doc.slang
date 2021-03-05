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


/// An interface to do things
interface IDoThing
{
        /// Add two integers
    int add(int a,      ///< First parameter 
        int b           ///< Second parameter
    );
    
        /// Subtract
        /// Multi-line
    int sub(int a,      ///< First
            int b       ///< Second
    );
}

interface IThing
{
    float getValue();
};

/// Implement IThing on float
extension float : IThing
{
        /// Just return the float itself!
	float getValue() { return this; }
}

struct Thing : IThing, IDoThing
{
    int add(int a, int b) { return a + b; }
    int sub(int a, int b ) { return a - b; }
    float getValue() { return 1.0f; }
};

/// A struct with some fields
struct SomeStruct
{
    /// A field
    int aField;
    /// Multi-line 
    /// is a thing
    int anotherField;
    int yetAnother; ///< A field with stuff 
    
        /// Get a value 
    int getMethod() { return yetAnother; }
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

/// A rather silly generic function to test out doc extraction
T addInts<T : __BuiltinIntegerType,     ///< The type we are operating on 
    /// Just testing out a 
    /// non type based generic
    let U : int,
    let V : int>    ///< And another one                       
    (
    /// CHECKING!!
    T z,           ///< The Z parameter
    T b)            ///< The B parameter
{
    return z + b;
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
