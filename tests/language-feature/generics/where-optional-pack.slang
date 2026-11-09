//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

interface ITagged
{
    int tag();
}

struct Tagged : ITagged
{
    int tag() { return 7; }
}

void appendTag<T>(T value, inout int code) where optional T : ITagged
{
    if (T is ITagged)
        code = code * 10 + value.tag();
    else
        code = code * 10;
}

int encode<each T>(expand each T values) where optional T : ITagged
{
    int code = 0;
    expand appendTag<each T>(each values, code);
    return code;
}

int forward<each T>(expand each T values) where optional T : ITagged
{
    return encode<expand each T>(expand each values);
}

struct Encoder<each T> where optional T : ITagged
{
    static int run(expand each T values)
    {
        return forward<expand each T>(expand each values);
    }
}

typealias Alias<each T> = Encoder<expand each T>;
typealias ConstrainedAlias<each T> where optional T : ITagged = Encoder<expand each T>;

int forwardUnconstrained<each T>(expand each T values)
{
    return encode<expand each T>(expand each values);
}

int forwardScalar<T>(T value)
{
    int code = 0;
    appendTag<T>(value, code);
    return code;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // A missing proof must not remove the proof for the other element.
    outputBuffer[0] = encode(Tagged(), uint(1)); // CHECK: 70
    outputBuffer[1] = encode(uint(1), Tagged()); // CHECK: 7
    outputBuffer[2] = encode(Tagged(), Tagged()); // CHECK: 77
    outputBuffer[3] = encode(uint(1), uint(2)); // CHECK: 0
    outputBuffer[4] = encode(); // CHECK: 0
    outputBuffer[5] = forward(Tagged(), uint(1)); // CHECK: 70
    // The unconstrained alias supplies absent evidence before it is specialized.
    outputBuffer[6] = Alias<uint, Tagged>.run(uint(1), Tagged()); // CHECK: 0
    outputBuffer[7] = Encoder<Tagged, uint>.run(Tagged(), uint(1)); // CHECK: 70

    // An unconstrained caller supplies no proof, including after concrete specialization.
    outputBuffer[8] = forwardScalar(Tagged()); // CHECK: 0
    outputBuffer[9] = forwardUnconstrained(Tagged(), uint(1)); // CHECK: 0
    outputBuffer[10] = forwardUnconstrained(); // CHECK: 0
    outputBuffer[11] = ConstrainedAlias<uint, Tagged>.run(uint(1), Tagged()); // CHECK: 7
}
