//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -conformance GeometryA:IGeometry=3 -conformance GeometryB:IGeometry=7

// Related to #13245: construction and consumption of an associated result stay
// together inside a concrete helper. The public-ID mapping is still necessary.
interface IHit
{
    uint evaluate();
}

struct HitA : IHit
{
    uint value;
    uint evaluate() { return value + 10; }
}

struct HitB : IHit
{
    uint2 value;
    uint evaluate() { return value.x + value.y; }
}

interface IGeometry
{
    associatedtype Hit : IHit;
    static Hit unpack(uint value);
}

struct GeometryA : IGeometry
{
    typedef HitA Hit;
    static Hit unpack(uint value) { return {value}; }
}

struct GeometryB : IGeometry
{
    typedef HitB Hit;
    static Hit unpack(uint value) { return {uint2(value, value * 2)}; }
}

[__specializePerConformance]
void evaluate<G : IGeometry>(G geometry, uint value, out uint result)
{
    let hit = G.unpack(value);
    result = hit.evaluate();
}

//TEST_INPUT: type_conformance GeometryA:IGeometry = 3
//TEST_INPUT: type_conformance GeometryB:IGeometry = 7
//TEST_INPUT:ubuffer(data=[3 7], stride=4):name=ids
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
StructuredBuffer<uint> ids;
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    evaluate(createDynamicObject<IGeometry>(ids[tid.x], 0u), tid.x + 1, outputBuffer[tid.x]);
}

// CHECK: 11
// CHECK: 6
// HLSL: #pragma pack_matrix
// HLSL-NOT: AnyValue
