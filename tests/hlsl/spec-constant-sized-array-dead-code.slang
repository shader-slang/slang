// An array sized by a specialization constant in code that is removed before emit is not an
// error on targets that cannot size an array that way (#14030).

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -warnings-disable 39019

// CHECK-NOT: E55219
// CHECK: [numthreads(1, 1, 1)]

[SpecializationConstant]
const int kTile = 4;

static const bool kUseTile = false;

RWStructuredBuffer<float> outputBuffer;

float sumTile()
{
    float tile[kTile];
    for (int i = 0; i < kTile; i++)
        tile[i] = i;
    return tile[0] + tile[kTile - 1];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float result = 1.0;
    if (kUseTile)
        result = sumTile();
    outputBuffer[0] = result;
}
