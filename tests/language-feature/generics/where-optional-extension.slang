// TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
// TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

interface ITagged
{
    int tag();
}

struct Tagged : ITagged
{
    int tag() { return 7; }
}

struct Box<T>
    where optional T : ITagged
{
}

extension<T>
    Box<T> where optional T : ITagged
{
    static int value(T value)
    {
        if (T is ITagged)
            return value.tag();
        else
            return 0;
    }
}

struct PackBox<each T> where optional T : ITagged
{
}

extension<each T>
    PackBox<expand each T> where optional T : ITagged
{
    static int sum(expand each T values)
    {
        int sum = 0;
        expand sum += Box<each T>.value(each values);
        return sum;
    }
}

struct NumericBox<T> where optional T : IDifferentiable
{
}

extension<T>
    NumericBox<T> where optional T : IDifferentiable
{
    static int hasDifferential()
    {
        if (T is IDifferentiable)
            return 1;
        else
            return 0;
    }
}

interface ITaggedPack<each T> where optional T : ITagged {}
struct TaggedPair : ITaggedPack<Tagged, uint> {}
struct NestedBox<T : ITagged, F : ITaggedPack<T, uint>> {}

extension<T : ITagged, F : ITaggedPack<T, uint>> NestedBox<T, F>
{
    static int value() { return 9; }
}

int nested<T : ITagged, F : ITaggedPack<T, uint>>()
{
    return NestedBox<T, F>.value();
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)] void computeMain()
{
    outputBuffer[0] = Box<uint>.value(1);                     // CHECK: 0
    outputBuffer[1] = Box<Tagged>.value(Tagged());            // CHECK: 7
    outputBuffer[2] = PackBox<uint, Tagged>.sum(1, Tagged()); // CHECK: 7
    outputBuffer[3] = PackBox<uint, uint>.sum(1, 2);          // CHECK: 0
    outputBuffer[4] = PackBox<>.sum();                        // CHECK: 0
    // Matching the extension must retain uint's chosen absence, rather than promote it to float.
    outputBuffer[5] = NumericBox<uint>.hasDifferential();  // CHECK: 0
    outputBuffer[6] = NumericBox<float>.hasDifferential(); // CHECK: 1
    outputBuffer[7] = nested<Tagged, TaggedPair>(); // CHECK: 9
}
