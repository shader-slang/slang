// Optional<T> of a concrete type as a field in interface implementations.
// Optional is lowered to a struct with hasValue + value before AnyValue
// marshalling. Tests both Some and None values through dynamic dispatch.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IPoint
{
    float3 getPosition();
    float getWeight();
    int hasNormal();
    float getNormalY();
}

struct WeightedPoint : IPoint
{
    float3 position;
    Optional<float3> normal;
    float weight;

    float3 getPosition() { return position; }
    float getWeight() { return weight; }
    int hasNormal() { return normal.hasValue ? 1 : 0; }

    float getNormalY()
    {
        if (normal.hasValue)
            return normal.value.y;
        return 0.0;
    }
}

struct SimplePoint : IPoint
{
    float3 position;
    Optional<float> scale;

    float3 getPosition() { return position; }
    float getWeight() { return scale.hasValue ? scale.value : 1.0; }
    int hasNormal() { return 0; }
    float getNormalY() { return 0.0; }
}

IPoint makePoint(int id)
{
    switch (id)
    {
    case 0:
    {
        WeightedPoint p;
        p.position = float3(1.0, 2.0, 3.0);
        p.normal = float3(0.0, 1.0, 0.0);
        p.weight = 0.5;
        return p;
    }
    case 1:
    {
        WeightedPoint p;
        p.position = float3(4.0, 5.0, 6.0);
        p.normal = none;
        p.weight = 0.75;
        return p;
    }
    default:
    {
        SimplePoint p;
        p.position = float3(7.0, 8.0, 9.0);
        p.scale = 2.0;
        return p;
    }
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // WeightedPoint with normal: pos=(1,2,3), normal=(0,1,0), weight=0.5
    var a = makePoint(id);
    outputBuffer[0] = a.getPosition().x;
    // CHECK: 1.0
    outputBuffer[1] = a.getWeight();
    // CHECK: 0.5
    outputBuffer[2] = float(a.hasNormal());
    // CHECK: 1.0
    outputBuffer[3] = a.getNormalY();
    // CHECK: 1.0

    // WeightedPoint without normal: pos=(4,5,6), normal=none, weight=0.75
    var b = makePoint(id + 1);
    outputBuffer[4] = b.getPosition().x;
    // CHECK: 4.0
    outputBuffer[5] = b.getWeight();
    // CHECK: 0.75
    outputBuffer[6] = float(b.hasNormal());
    // CHECK: 0.0
    outputBuffer[7] = b.getNormalY();
    // CHECK: 0.0

    // SimplePoint: pos=(7,8,9), scale=2.0
    var c = makePoint(id + 2);
    outputBuffer[8] = c.getPosition().x;
    // CHECK: 7.0
    outputBuffer[9] = c.getWeight();
    // CHECK: 2.0
}
