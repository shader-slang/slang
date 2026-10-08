//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

Texture2D<float> texture;

[numthreads(1, 1, 1)]
void computeMain()
//CHECK: error[E36107]: unavailable features in entry point
{
    uint width;
    uint height;
    uint numberOfLevels;
    texture.GetDimensions(1, width, height, numberOfLevels);
}
