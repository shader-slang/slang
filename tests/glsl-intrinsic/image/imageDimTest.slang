//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 4) in;

//TEST_INPUT: RWTexture1D(format=R16G16B16A16_UINT, size=2, content=one, mipMaps = 1):name i_1d
uniform layout(binding=0,rgba16ui) writeonly uimage1D i_1d;
//TEST_INPUT: RWTextureBuffer(format=R16G16B16A16_UINT, stride=8, data=[0x00010001 0x00010001 0x00010001 0x00010001]):name i_ib
uniform layout(binding=1,rgba16ui) writeonly uimageBuffer i_ib;

//TEST_INPUT: RWTexture1D(format=R16G16B16A16_UINT, size=2, content=one, mipMaps = 1, arrayLength=2):name i_1da
uniform layout(binding=2,rgba16ui) writeonly uimage1DArray i_1da;
//TEST_INPUT: RWTexture2D(format=R16G16B16A16_UINT, size=2, content=one, mipMaps = 1):name i_2d
uniform layout(binding=3,rgba16ui) writeonly uimage2D i_2d;
//TEST_INPUT: RWTexture2D(format=R16G16B16A16_UINT, size=2, content=one, mipMaps = 1, arrayLength=2):name i_2da
uniform layout(binding=4,rgba16ui) writeonly uimage2DArray i_2da;
//TEST_INPUT: RWTexture2D(format=R16G16B16A16_UINT, size=2, content=one, mipMaps = 1):name i_2dr
uniform layout(binding=5,rgba16ui) writeonly uimage2DRect i_2dr;
//TEST_INPUT: RWTexture2D(format=R16G16B16A16_UINT, size=2, content=one, mipMaps = 1, sampleCount = two):name i_2dms
uniform layout(binding=6,rgba16ui) writeonly uimage2DMS i_2dms;

//TEST_INPUT: RWTexture3D(format=R16G16B16A16_UINT, size=2, content=one, mipMaps = 1):name i_3d
uniform layout(binding=7,rgba16ui) writeonly uimage3D i_3d;
//TEST_INPUT: RWTextureCube(format=R16G16B16A16_UINT, size=2, content=one, mipMaps = 1):name i_c
uniform layout(binding=8,rgba16ui) writeonly uimageCube i_c;
//TEST_INPUT: RWTextureCube(format=R16G16B16A16_UINT, size=2, content=one, mipMaps = 1, arrayLength=2):name i_ca
uniform layout(binding=9,rgba16ui) writeonly uimageCubeArray i_ca;
//TEST_INPUT: RWTexture2D(format=R16G16B16A16_UINT, size=2, content=one, mipMaps = 1, arrayLength=2, sampleCount = two):name i_2dmsa
uniform layout(binding=10,rgba16ui) writeonly uimage2DMSArray i_2dmsa;

bool checkAllImageSize()
{
    return true
        && imageSize(i_1d) == int(2)
        && imageSize(i_ib) == int(2)
        && imageSize(i_1da) == ivec2(2)
        && imageSize(i_2d) == ivec2(2)
        && imageSize(i_2da) == ivec3(2)
        && imageSize(i_2dr) == ivec2(2)
        && imageSize(i_2dms) == ivec2(2)
        && imageSize(i_3d) == ivec3(2)
        && imageSize(i_c) == ivec2(2)
        && imageSize(i_ca) == ivec3(2)
        && imageSize(i_2dmsa) == ivec3(2)
        ;
}

// requires ImageLoad test to pass 
bool checkAllImageStore()
{
    bool loadCheck = true;

    imageStore(i_1d,0,uvec4(0));
    loadCheck = loadCheck && imageLoad(i_1d,0) == uvec4(0);
    imageStore(i_1d,0,uvec4(1));
    
    imageStore(i_ib,0,uvec4(0));
    loadCheck = loadCheck && loadCheck && imageLoad(i_ib,0) == uvec4(0);
    imageStore(i_ib,0,uvec4(1));


    imageStore(i_1da,ivec2(0),uvec4(0));
    loadCheck = loadCheck && imageLoad(i_1da,0) == uvec4(0);
    imageStore(i_1da,ivec2(0),uvec4(1));

    imageStore(i_2d,ivec2(0),uvec4(0));
    loadCheck = loadCheck && imageLoad(i_2d,ivec2(0)) == uvec4(0);
    imageStore(i_2d,ivec2(0),uvec4(1));

    imageStore(i_2dr,ivec2(0),uvec4(0));
    loadCheck = loadCheck && imageLoad(i_2dr,ivec2(0)) == uvec4(0);
    imageStore(i_2dr,ivec2(0),uvec4(1));

    imageStore(i_2dms,ivec2(0), 1, uvec4(0));
    loadCheck = loadCheck && imageLoad(i_2dms,ivec2(0),1) == uvec4(0);
    imageStore(i_2dms,ivec2(0), 1, uvec4(1));


    imageStore(i_2da,ivec3(0),uvec4(0));
    loadCheck = loadCheck && imageLoad(i_2da,ivec3(0)) == uvec4(0);
    imageStore(i_2da,ivec3(0),uvec4(1));

    imageStore(i_3d,ivec3(0),uvec4(0));
    loadCheck = loadCheck && imageLoad(i_3d,ivec3(0)) == uvec4(0);
    imageStore(i_3d,ivec3(0),uvec4(1));

    imageStore(i_c,ivec3(0),uvec4(0));
    loadCheck = loadCheck && imageLoad(i_c,ivec3(0)) == uvec4(0);
    imageStore(i_c,ivec3(0),uvec4(1));

    imageStore(i_ca,ivec3(0),uvec4(0));
    loadCheck = loadCheck && imageLoad(i_ca,ivec3(0)) == uvec4(0);
    imageStore(i_ca,ivec3(0),uvec4(1));

    imageStore(i_2dmsa,ivec3(0),1,uvec4(0));
    loadCheck = loadCheck && imageLoad(i_2dmsa,ivec3(0), 1) == uvec4(0);
    imageStore(i_2dmsa,ivec3(0),1,uvec4(1));
    return loadCheck;
        ;
}
bool checkAllImageLoad()
{
    return true
        && imageLoad(i_1d,0) == uvec4(1)
        && imageLoad(i_ib,0) == uvec4(1)

        && imageLoad(i_1da,ivec2(0)) == uvec4(1)
        && imageLoad(i_2d,ivec2(0)) == uvec4(1)
        && imageLoad(i_2dr,ivec2(0)) == uvec4(1)
        && imageLoad(i_2dms,ivec2(0),1) == uvec4(1)
        
        && imageLoad(i_2da,ivec3(0)) == uvec4(1)
        && imageLoad(i_3d,ivec3(0)) == uvec4(1)
        && imageLoad(i_c,ivec3(0)) == uvec4(1)
        && imageLoad(i_ca,ivec3(0)) == uvec4(1)
        && imageLoad(i_ca,ivec3(0)) == uvec4(1)
        && imageLoad(i_2dmsa,ivec3(0),1) == uvec4(1)
        ;
}

bool imageOperations()
{
    return true
        && checkAllImageSize()
        && checkAllImageLoad()
        && checkAllImageStore();
        ;
}

// CHECK_GLSL: void main(
// CHECK_SPV: OpEntryPoint
void computeMain()
{
    outputBuffer.data[0] = true
        && imageOperations()
        ;
    // BUF: 1
}
