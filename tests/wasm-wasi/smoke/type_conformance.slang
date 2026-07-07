// An interface with three implementing types, used by smoke-test.py to
// exercise slang_wasm_type_conformances_* (ISession::createTypeConformanceComponentType).

interface IMaterial
{
    int apply(int x);
}

struct AMaterial : IMaterial
{
    int apply(int x) { return x + 1; }
}

struct BMaterial : IMaterial
{
    int apply(int x) { return x + 2; }
}

struct CMaterial : IMaterial
{
    int apply(int x) { return x + 3; }
}

ParameterBlock<IMaterial> material;
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = material.apply((int)tid.x);
}
