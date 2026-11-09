//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

public struct Pair
{
    public float first;
    public float second;
}

public struct Result
{
    public float scalar;
    public Pair pair;
}

[host]
Result main(RWStructuredBuffer<float> scalars, RWStructuredBuffer<Pair> pairs)
{
    return Result(readBack(scalars, 0), pairs.readBack(0));
}

// CHECK-COUNT-2: OpReadBackBufferElement
