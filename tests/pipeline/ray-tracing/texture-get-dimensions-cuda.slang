//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry raygenMain -stage raygeneration
//TEST(optix):COMPARE_COMPUTE_EX:-cuda -rt -output-using-type -compute-dispatch 6,1,1

//CHECK: txq.level.width.b32 %0, [%1], %2;
//CHECK-SAME: "r"((1U))
//CHECK: txq.level.height.b32 %1, [%2], %3;
//CHECK: txq.level.depth.b32 %2, [%3], %4;
//CHECK: txq.num_mipmap_levels.b32 %0, [%1];

//TEST_INPUT: Texture1D(size=4, content=one, mipMaps=2):name texture1D
Texture1D<float> texture1D;
//TEST_INPUT: Texture2D(size=8, content=one, mipMaps=2):name texture2D
Texture2D<float> texture2D;
//TEST_INPUT: Texture3D(size=2, content=one, mipMaps=2):name texture3D
Texture3D<float> texture3D;
//TEST_INPUT: TextureCube(size=16, content=one, mipMaps=2):name textureCube
TextureCube<float> textureCube;
//TEST_INPUT: Texture2D(size=4, content=one, arrayLength=3, mipMaps=2):name texture2DArray
Texture2DArray<float> texture2DArray;
//TEST_INPUT: TextureCube(size=16, content=one, arrayLength=1, mipMaps=2):name textureCubeArray
TextureCubeArray<float> textureCubeArray;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void raygenMain()
{
    uint idx = DispatchRaysIndex().x;
    uint width;
    uint height;
    uint depth;
    // Verify that array overloads overwrite this unsupported output with zero.
    uint elements = 0xffffffff;
    // This is read after the switch, so initialize the unmatched-index path.
    uint numberOfLevels = 0;

    switch (idx)
    {
        case 0:
            texture1D.GetDimensions(1, width, numberOfLevels);
            output[idx] = width;
            break;
        case 1:
            texture2D.GetDimensions(1, width, height, numberOfLevels);
            output[idx] = ((0xff & width) << 8) | (0xff & height);
            break;
        case 2:
            texture3D.GetDimensions(1, width, height, depth, numberOfLevels);
            output[idx] = ((0xff & width) << 16) | ((0xff & height) << 8) | (0xff & depth);
            break;
        case 3:
            textureCube.GetDimensions(1, width, height, numberOfLevels);
            output[idx] = ((0xff & width) << 8) | (0xff & height);
            break;
        case 4:
            texture2DArray.GetDimensions(1, width, height, elements, numberOfLevels);
            output[idx] = ((0xff & width) << 8) | (0xff & height);
            output[12] = elements;
            break;
        case 5:
            textureCubeArray.GetDimensions(1, width, height, elements, numberOfLevels);
            output[idx] = ((0xff & width) << 8) | (0xff & height);
            output[13] = elements;
            break;
    }
    output[idx + 6] = numberOfLevels;
}
