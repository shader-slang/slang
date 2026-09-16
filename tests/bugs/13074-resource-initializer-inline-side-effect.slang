//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage compute -validate-ir -warnings-disable 41024

//CHECK-LABEL: void _S{{[0-9]+}}()
//CHECK: [[COUNT:initializationCount_[0-9]+]] = [[COUNT]] + 1U;
//CHECK-LABEL: void main()
//CHECK: _S{{[0-9]+}}();

static uint initializationCount = 0;
Texture2D<float4> texture;
RWStructuredBuffer<float4> output;

static Texture2D<float4> cachedTexture = (++initializationCount, texture);

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0)) + initializationCount;
}
