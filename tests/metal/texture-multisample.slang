//TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal -DEMIT_SOURCE

// TEST_INPUT: RWTexture2D(size=4, content = zero, sampleCount=2):name texMS
Texture2DMS<float4> texMS;
// TEST_INPUT: RWTexture2D(size=4, content = zero):name output
RWTexture2D<float4> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    int2 location = int2(0, 0);
    uint sampleIndex = 0;
    // METAL: {{.*}}access::read{{.*}}
    float4 v = texMS.Load(location, sampleIndex);
    output[location] = v;
}