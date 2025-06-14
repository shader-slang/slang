//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IReleaseable
{
    [mutating]
    void release();
}

struct Container<K, V>
    where optional K : IReleaseable
    where optional V : IReleaseable
{
    K k[2];
    V v[2];

    [mutating]
    void erase(int index)
    {
        if (K is IReleaseable)
            k[index].release();
        if (V is IReleaseable)
            v[index].release();
    }
}

struct HeavyEntry: IReleaseable
{
    int index;
    int value;

    [mutating]
    void release()
    {
        outputBuffer[index] = value;
    }
};

struct LightEntry
{
    int value;
};

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    { // Neither is IReleaseable
        var c = Container<LightEntry, LightEntry>();
        c.k[0] = LightEntry(1);
        c.k[1] = LightEntry(2);
        c.v[0] = LightEntry(3);
        c.v[1] = LightEntry(4);
        c.erase(0);
        c.erase(1);
    }
    { // K is IReleaseable
        var c = Container<HeavyEntry, LightEntry>();
        c.k[0] = HeavyEntry(0,1);
        c.k[1] = HeavyEntry(1,2);
        c.v[0] = LightEntry(3);
        c.v[1] = LightEntry(4);
        // CHECK: 1
        c.erase(0);
        // CHECK-NEXT: 2
        c.erase(1);
    }
    { // V is IReleaseable
        var c = Container<LightEntry, HeavyEntry>();
        c.k[0] = LightEntry(1);
        c.k[1] = LightEntry(2);
        c.v[0] = HeavyEntry(2,3);
        c.v[1] = HeavyEntry(3,4);
        // CHECK-NEXT: 3
        c.erase(0);
        // CHECK-NEXT: 4
        c.erase(1);
    }
    { // K and V are IReleaseable
        var c = Container<HeavyEntry, HeavyEntry>();
        c.k[0] = HeavyEntry(4,5);
        c.k[1] = HeavyEntry(6,7);
        c.v[0] = HeavyEntry(5,6);
        c.v[1] = HeavyEntry(7,8);
        // CHECK-NEXT: 5
        // CHECK-NEXT: 6
        c.erase(0);
        // CHECK-NEXT: 7
        // CHECK-NEXT: 8
        c.erase(1);
    }
}
