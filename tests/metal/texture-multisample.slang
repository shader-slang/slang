//TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal -DEMIT_SOURCE
//DISABLE_TEST:SIMPLE(filecheck=METALLIB): -stage compute -entry computeMain -target metallib -DEMIT_SOURCE

// Disabled due to location argument being passed wrong argument type
// TODO: github issue #8457

// METALLIB-NOT: error :
// METALLIB: @computeMain

Texture2DMS<float4> texMS;
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
