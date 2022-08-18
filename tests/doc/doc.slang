//TEST:DOC:-entry computeMain -target hlsl -stage compute -doc -no-codegen

void outFunc(out int v)
{
    v = 10;
}

/// Testing out nested generics
struct ParentStruct<T> ///< Some type
{
    /// Testing out a child
    struct ChildStruct<S> ///< Some other type
    {
        /// A useless method hey ho
        T getValue(S v) { T t; S s; return t; }
    };
};

struct GenericStruct<T>
{
        /// Let's try a typedef too
    typedef T Element;

    T getValue() { return value; }
    
    T value;
};

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

namespace Hey
{
    void doAnotherThing(int a);
}

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
        /// An associated type
    associatedtype V;
    
        /// Add two integers
    V add(V a,      ///< First parameter 
        V b           ///< Second parameter
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
    typedef int V;
    
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


[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int a = int(dispatchThreadID.x);
    int b = int(dispatchThreadID.y);
    int c = int(dispatchThreadID.z);
    int d = a + b * c;
    int e = d + c / 2;
    
    for (int i = 0; i < b; ++i)
    {
        if (e > 10 && (i & 2) != 0)
        {
            a += b; b -= c; c += c; d = d + e + a; e = a;
        }
        else
        {
            a = e; b = c + c; d += d + __SyntaxError(); e = doThing(e, int(dispatchThreadID.x));
            
        }
    }
    
    outputBuffer[dispatchThreadID.x] = a + b + c + d + e;
}
