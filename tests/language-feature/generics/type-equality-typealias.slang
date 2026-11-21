//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Value
{
    int payload;
}

typealias Alias = Value;

interface IHasValue
{
    associatedtype Item;
}

struct Container : IHasValue
{
    typealias Item = Value;
}

int readGeneric<T>(T value) where T == Alias
{
    return value.payload;
}

int readAssociated<T : IHasValue>(T.Item value) where T.Item == Alias
{
    return value.payload;
}

RWStructuredBuffer<int> outputBuffer;

// CHECK: void main
[numthreads(1, 1, 1)]
void main()
{
    Value value = { 7 };
    outputBuffer[0] = readGeneric(value);
    outputBuffer[1] = readAssociated<Container>(value);
}
