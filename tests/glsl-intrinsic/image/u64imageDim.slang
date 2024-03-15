//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly

// As per vulkan specification: https://registry.khronos.org/vulkan/specs/1.3/html/chap34.html#formats-definition
// i64/u64 images do not require sampled image support; support is currently unlikley on hardware
//#define test_when_hardware_supports_i64_and_u64_sampled_textures
// i64/u64 images do not require texel buffers support; support is currently unlikley on hardware
//#define TEST_when_hardware_supports_i64_and_u64_buffers

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 1) in;

//TEST_INPUT: RWTexture1D(format=R64_UINT, size=4, content=one, mipMaps = 1):name i_1d
uniform layout(binding=0,r64ui) u64image1D i_1d;
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
//COM:TEST_INPUT: RWTextureBuffer(format=R64_UINT, stride=8, data=[0 1 0 1 0 1 0 1]):name i_ib
uniform layout(binding=1,r64ui) u64imageBuffer i_ib;
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
//TEST_INPUT: RWTexture1D(format=R64_UINT, size=4, content=one, mipMaps = 1, arrayLength=2):name i_1da
uniform layout(binding=2,r64ui) u64image1DArray i_1da;
//TEST_INPUT: RWTexture2D(format=R64_UINT, size=4, content=one, mipMaps = 1):name i_2d
uniform layout(binding=3,r64ui) u64image2D i_2d;
//TEST_INPUT: RWTexture2D(format=R64_UINT, size=4, content=one, mipMaps = 1):name i_2dr
uniform layout(binding=4,r64ui) u64image2DRect i_2dr;
//TEST_INPUT: RWTexture2D(format=R64_UINT, size=4, content=one, mipMaps = 1, arrayLength=2):name i_2da
uniform layout(binding=6,r64ui) u64image2DArray i_2da;
//TEST_INPUT: RWTexture3D(format=R64_UINT, size=4, content=one, mipMaps = 1):name i_3d
uniform layout(binding=7,r64ui) u64image3D i_3d;
//TEST_INPUT: RWTextureCube(format=R64_UINT, size=4, content=one, mipMaps = 1):name i_c
uniform layout(binding=8,r64ui) u64imageCube i_c;
//TEST_INPUT: RWTextureCube(format=R64_UINT, size=4, content=one, mipMaps = 1, arrayLength=2):name i_ca
uniform layout(binding=9,r64ui) u64imageCubeArray i_ca;
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures  
//COM:TEST_INPUT: RWTexture2D(format=R64_UINT, size=4, content=one, mipMaps = 1, sampleCount = two):name i_2dms
uniform layout(binding=5,r64ui) u64image2DMS i_2dms;
//COM:TEST_INPUT: RWTexture2D(format=R64_UINT, size=4, content=one, mipMaps = 1, arrayLength=2, sampleCount = two):name i_2dmsa
uniform layout(binding=10,r64ui) u64image2DMSArray i_2dmsa;
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures  

bool checkAllImageSize()
{
    return true
        && imageSize(i_1d) == int(4)
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageSize(i_ib) == int(4)
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageSize(i_1da) == ivec2(4, 2)
        && imageSize(i_2d) == ivec2(4)
        && imageSize(i_2da) == ivec3(4, 4, 2)
        && imageSize(i_2dr) == ivec2(4)
        && imageSize(i_3d) == ivec3(4)
        && imageSize(i_c) == ivec2(4)
        && imageSize(i_ca) == ivec3(4, 4, 2)
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageSize(i_2dms) == ivec2(4)
        && imageSize(i_2dmsa) == ivec3(4, 4, 2)
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool checkAllImageLoad()
{
    return true
        && imageLoad(i_1d, 0).x == u64vec4(1).x
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageLoad(i_ib, 0).x == u64vec4(1).x
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageLoad(i_1da, ivec2(0)).x == u64vec4(1).x
        && imageLoad(i_2d, ivec2(0)).x == u64vec4(1).x
        && imageLoad(i_2dr, ivec2(0)).x == u64vec4(1).x
        && imageLoad(i_2da, ivec3(0)).x == u64vec4(1).x
        && imageLoad(i_3d, ivec3(0)).x == u64vec4(1).x
        && imageLoad(i_c, ivec3(0)).x == u64vec4(1).x
        && imageLoad(i_ca, ivec3(0)).x == u64vec4(1).x
        && imageLoad(i_ca, ivec3(0)).x == u64vec4(1).x
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageLoad(i_2dms, ivec2(0), 1).x == u64vec4(1).x
        && imageLoad(i_2dmsa, ivec3(0), 1).x == u64vec4(1).x
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool resetAllImageValues()
{
    imageStore(i_1d, 0,u64vec4(1));
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
    imageStore(i_ib, 0,u64vec4(1));
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
    imageStore(i_1da, ivec2(0),u64vec4(1));
    imageStore(i_2d, ivec2(0),u64vec4(1));
    imageStore(i_2dr, ivec2(0),u64vec4(1));
    imageStore(i_2da, ivec3(0),u64vec4(1));
    imageStore(i_3d, ivec3(0),u64vec4(1));
    imageStore(i_c, ivec3(0),u64vec4(1));
    imageStore(i_ca, ivec3(0),u64vec4(1));
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
    imageStore(i_2dms, ivec2(0), 1, u64vec4(1));
    imageStore(i_2dmsa, ivec3(0), 1,u64vec4(1));
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
    return true;
}
uint64_t load_1d()
{
    return imageLoad(i_1d, 0).x;
}
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
uint64_t load_ib()
{
    return imageLoad(i_ib, 0).x;
}
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
uint64_t load_1da()
{
    return imageLoad(i_1da, ivec2(0)).x;
}
uint64_t load_2d()
{
    return imageLoad(i_2d, ivec2(0)).x;
}
uint64_t load_2dr()
{
    return imageLoad(i_2dr, ivec2(0)).x;
}
uint64_t load_2da()
{
    return imageLoad(i_2da, ivec3(0)).x;
}
uint64_t load_3d()
{
    return imageLoad(i_3d, ivec3(0)).x; 
}
uint64_t load_c()
{
    return imageLoad(i_c, ivec3(0)).x;
}
uint64_t load_ca()
{
    return imageLoad(i_ca, ivec3(0)).x;     
}
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
uint64_t load_2dms()
{
    return imageLoad(i_2dms, ivec2(0), 1).x;
}
uint64_t load_2dmsa()
{
    return imageLoad(i_2dmsa, ivec3(0), 1).x;      
}
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures

// requires ImageLoad test to pass 
bool checkAllImageStore()
{
    bool loadCheck = true;

    imageStore(i_1d, 0,u64vec4(0));
    loadCheck = loadCheck && load_1d() == 0;
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
    imageStore(i_ib, 0,u64vec4(0));
    loadCheck = loadCheck && load_ib() == 0;
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
    imageStore(i_1da, ivec2(0),u64vec4(0));
    loadCheck = loadCheck && load_1da() == 0;
    imageStore(i_2d, ivec2(0),u64vec4(0));
    loadCheck = loadCheck && load_2d() == 0;
    imageStore(i_2dr, ivec2(0),u64vec4(0));
    loadCheck = loadCheck && load_2dr() == 0;
    imageStore(i_2da, ivec3(0),u64vec4(0));
    loadCheck = loadCheck && load_2da() == 0;
    imageStore(i_3d, ivec3(0),u64vec4(0));
    loadCheck = loadCheck && load_3d() == 0;
    imageStore(i_c, ivec3(0),u64vec4(0));
    loadCheck = loadCheck && load_c() == 0;
    imageStore(i_ca, ivec3(0),u64vec4(0));
    loadCheck = loadCheck && load_ca() == 0;
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
    imageStore(i_2dms, ivec2(0), 1, u64vec4(0));
    loadCheck = loadCheck && load_2dms() == 0;
    imageStore(i_2dmsa, ivec3(0), 1,u64vec4(0));
    loadCheck = loadCheck && load_2dmsa() == 0;
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
    resetAllImageValues();
    return loadCheck;
        ;
}
bool checkAllImageSamples()
{
    resetAllImageValues();
    return true
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageSamples(i_2dms) == 2
        && imageSamples(i_2dmsa) == 2
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool checkAllImageAtomicAdd()
{
    resetAllImageValues();
    return true
        && imageAtomicAdd(i_1d, 0, 0) == 1
        && load_1d() == 1
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicAdd(i_ib, 0, 2) == 1
        && load_ib() == 3
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicAdd(i_1da, ivec2(0), 0) == 1
        && load_1da() == 1
        && imageAtomicAdd(i_2d, ivec2(0), 2) == 1
        && load_2d() == 3
        && imageAtomicAdd(i_2dr, ivec2(0), 2) == 1
        && load_2dr() == 3
        && imageAtomicAdd(i_2da, ivec3(0), 0) == 1
        && load_2da() == 1
        && imageAtomicAdd(i_3d, ivec3(0), 2) == 1
        && load_3d() == 3
        && imageAtomicAdd(i_c, ivec3(0), 2) == 1
        && load_c() == 3
        && imageAtomicAdd(i_ca, ivec3(0), 2) == 1
        && load_ca() == 3
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageAtomicAdd(i_2dms, ivec2(0), 1, 2) == 1
        && load_2dms() == 3
        && imageAtomicAdd(i_2dmsa, ivec3(0), 1, 2) == 1
        && load_2dmsa() == 3
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool checkAllImageAtomicExchange()
{
    resetAllImageValues();
    return true
        && imageAtomicExchange(i_1d, 0, 0) == 1
        && load_1d() == 0
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicExchange(i_ib, 0, 2) == 1
        && load_ib() == 2
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicExchange(i_1da, ivec2(0), 0) == 1
        && load_1da() == 0
        && imageAtomicExchange(i_2d, ivec2(0), 2) == 1
        && load_2d() == 2
        && imageAtomicExchange(i_2dr, ivec2(0), 2) == 1
        && load_2dr() == 2
        && imageAtomicExchange(i_2da, ivec3(0), 0) == 1
        && load_2da() == 0
        && imageAtomicExchange(i_3d, ivec3(0), 2) == 1
        && load_3d() == 2
        && imageAtomicExchange(i_c, ivec3(0), 2) == 1
        && load_c() == 2
        && imageAtomicExchange(i_ca, ivec3(0), 2) == 1
        && load_ca() == 2
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageAtomicExchange(i_2dms, ivec2(0), 1, 2) == 1
        && load_2dms() == 2
        && imageAtomicExchange(i_2dmsa, ivec3(0), 1, 2) == 1
        && load_2dmsa() == 2
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool checkAllImageAtomicMin()
{
    resetAllImageValues();
    return true
        && imageAtomicMin(i_1d, 0, 0) == 1
        && load_1d() == 0
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicMin(i_ib, 0, 2) == 1
        && load_ib() == 1
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicMin(i_1da, ivec2(0), 0) == 1
        && load_1da() == 0
        && imageAtomicMin(i_2d, ivec2(0), 2) == 1
        && load_2d() == 1
        && imageAtomicMin(i_2dr, ivec2(0), 2) == 1
        && load_2dr() == 1
        && imageAtomicMin(i_2da, ivec3(0), 0) == 1
        && load_2da() == 0
        && imageAtomicMin(i_3d, ivec3(0), 2) == 1
        && load_3d() == 1
        && imageAtomicMin(i_c, ivec3(0), 2) == 1
        && load_c() == 1
        && imageAtomicMin(i_ca, ivec3(0), 2) == 1
        && load_ca() == 1
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageAtomicMin(i_2dms, ivec2(0), 1, 2) == 1
        && load_2dms() == 1
        && imageAtomicMin(i_2dmsa, ivec3(0), 1, 2) == 1
        && load_2dmsa() == 1
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool checkAllImageAtomicMax()
{
    resetAllImageValues();
    return true
        && imageAtomicMax(i_1d, 0, 0) == 1
        && load_1d() == 1
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicMax(i_ib, 0, 2) == 1
        && load_ib() == 2
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicMax(i_1da, ivec2(0), 0) == 1
        && load_1da() == 1
        && imageAtomicMax(i_2d, ivec2(0), 2) == 1
        && load_2d() == 2
        && imageAtomicMax(i_2dr, ivec2(0), 2) == 1
        && load_2dr() == 2
        && imageAtomicMax(i_2da, ivec3(0), 0) == 1
        && load_2da() == 1
        && imageAtomicMax(i_3d, ivec3(0), 2) == 1
        && load_3d() == 2
        && imageAtomicMax(i_c, ivec3(0), 2) == 1
        && load_c() == 2
        && imageAtomicMax(i_ca, ivec3(0), 2) == 1
        && load_ca() == 2
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageAtomicMax(i_2dms, ivec2(0), 1, 2) == 1
        && load_2dms() == 2
        && imageAtomicMax(i_2dmsa, ivec3(0), 1, 2) == 1
        && load_2dmsa() == 2
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool checkAllImageAtomicAnd()
{
    resetAllImageValues();
    return true
        && imageAtomicAnd(i_1d, 0, 1) == 1
        && load_1d() == 1
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicAnd(i_ib, 0, 2) == 1
        && load_ib() == 0
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicAnd(i_1da, ivec2(0), 1) == 1
        && load_1da() == 1
        && imageAtomicAnd(i_2d, ivec2(0), 2) == 1
        && load_2d() == 0
        && imageAtomicAnd(i_2dr, ivec2(0), 2) == 1
        && load_2dr() == 0
        && imageAtomicAnd(i_2da, ivec3(0), 1) == 1
        && load_2da() == 1
        && imageAtomicAnd(i_3d, ivec3(0), 2) == 1
        && load_3d() == 0
        && imageAtomicAnd(i_c, ivec3(0), 2) == 1
        && load_c() == 0
        && imageAtomicAnd(i_ca, ivec3(0), 2) == 1
        && load_ca() == 0
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageAtomicAnd(i_2dms, ivec2(0), 1, 2) == 1
        && load_2dms() == 0
        && imageAtomicAnd(i_2dmsa, ivec3(0), 1, 2) == 1
        && load_2dmsa() == 0
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool checkAllImageAtomicOr()
{
    resetAllImageValues();
    return true
        && imageAtomicOr(i_1d, 0, 1) == 1
        && load_1d() == 1
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicOr(i_ib, 0, 2) == 1
        && load_ib() == 3
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicOr(i_1da, ivec2(0), 1) == 1
        && load_1da() == 1
        && imageAtomicOr(i_2d, ivec2(0), 2) == 1
        && load_2d() == 3
        && imageAtomicOr(i_2dr, ivec2(0), 2) == 1
        && load_2dr() == 3
        && imageAtomicOr(i_2da, ivec3(0), 1) == 1
        && load_2da() == 1
        && imageAtomicOr(i_3d, ivec3(0), 2) == 1
        && load_3d() == 3
        && imageAtomicOr(i_c, ivec3(0), 2) == 1
        && load_c() == 3
        && imageAtomicOr(i_ca, ivec3(0), 2) == 1
        && load_ca() == 3
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageAtomicOr(i_2dms, ivec2(0), 1, 2) == 1
        && load_2dms() == 3
        && imageAtomicOr(i_2dmsa, ivec3(0), 1, 2) == 1
        && load_2dmsa() == 3
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool checkAllImageAtomicXor()
{
    resetAllImageValues();
    return true
        && imageAtomicXor(i_1d, 0, 1) == 1
        && load_1d() == 0
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicXor(i_ib, 0, 2) == 1
        && load_ib() == 3
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicXor(i_1da, ivec2(0), 1) == 1
        && load_1da() == 0
        && imageAtomicXor(i_2d, ivec2(0), 2) == 1
        && load_2d() == 3
        && imageAtomicXor(i_2dr, ivec2(0), 2) == 1
        && load_2dr() == 3
        && imageAtomicXor(i_2da, ivec3(0), 1) == 1
        && load_2da() == 0
        && imageAtomicXor(i_3d, ivec3(0), 2) == 1
        && load_3d() == 3
        && imageAtomicXor(i_c, ivec3(0), 2) == 1
        && load_c() == 3
        && imageAtomicXor(i_ca, ivec3(0), 2) == 1
        && load_ca() == 3
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageAtomicXor(i_2dms, ivec2(0), 1, 2) == 1
        && load_2dms() == 3
        && imageAtomicXor(i_2dmsa, ivec3(0), 1, 2) == 1
        && load_2dmsa() == 3
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool checkAllImageAtomicCompSwap()
{
    resetAllImageValues();
    return true
        && imageAtomicCompSwap(i_1d, 0, 0, 2) == 1
        && load_1d() == 1
#ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicCompSwap(i_ib, 0, 1, 2) == 1
        && load_ib() == 2
#endif // #ifdef TEST_when_hardware_supports_i64_and_u64_buffers
        && imageAtomicCompSwap(i_1da, ivec2(0), 0, 2) == 1
        && load_1da() == 1
        && imageAtomicCompSwap(i_2d, ivec2(0), 1, 2) == 1
        && load_2d() == 2
        && imageAtomicCompSwap(i_2dr, ivec2(0), 1, 2) == 1
        && load_2dr() == 2
        && imageAtomicCompSwap(i_2da, ivec3(0), 0, 2) == 1
        && load_2da() == 1
        && imageAtomicCompSwap(i_3d, ivec3(0), 1, 2) == 1
        && load_3d() == 2
        && imageAtomicCompSwap(i_c, ivec3(0), 1, 2) == 1
        && load_c() == 2
        && imageAtomicCompSwap(i_ca, ivec3(0), 1, 2) == 1
        && load_ca() == 2
#ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        && imageAtomicCompSwap(i_2dms, ivec2(0), 1, 1, 2) == 1
        && load_2dms() == 2
        && imageAtomicCompSwap(i_2dmsa, ivec3(0), 1, 1, 2) == 1
        && load_2dmsa() == 2
#endif // #ifdef test_when_hardware_supports_i64_and_u64_sampled_textures
        ;
}
bool imageOperations()
{
    return true
        && checkAllImageSize()
        //&& checkAllImageLoad()
        //&& checkAllImageStore()
        //&& checkAllImageSamples()
        //&& checkAllImageAtomicAdd()
        //&& checkAllImageAtomicExchange()
        //&& checkAllImageAtomicMin()
        //&& checkAllImageAtomicMax()
        //&& checkAllImageAtomicAnd()
        //&& checkAllImageAtomicOr()
        //&& checkAllImageAtomicXor()
        //&& checkAllImageAtomicCompSwap()
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
