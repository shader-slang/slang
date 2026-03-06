// Enums with non-default underlying types (uint8_t, uint16_t, uint64_t)
// are marshalled via their tag type. The AnyValue packing must handle
// the different storage sizes correctly -- a uint8_t enum occupies 1 byte,
// while a uint64_t enum occupies 8 bytes in the packed representation.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -render-feature int16
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum TinyEnum : uint8_t
{
    Off = 0,
    Low = 1,
    High = 2,
}

enum SmallEnum : uint16_t
{
    None = 0,
    Some = 100,
    Full = 200,
}

enum DefaultEnum
{
    Alpha = 1000,
    Beta = 2000,
    Gamma = 3000,
}

interface IConfig
{
    int getTiny();
    int getSmall();
    int getDefault();
    int total();
}

struct FullConfig : IConfig
{
    TinyEnum tiny;
    SmallEnum small;
    DefaultEnum def;

    int getTiny() { return int(tiny); }
    int getSmall() { return int(small); }
    int getDefault() { return int(def); }
    int total() { return int(tiny) + int(small) + int(def); }
}

struct MinimalConfig : IConfig
{
    TinyEnum tiny;

    int getTiny() { return int(tiny); }
    int getSmall() { return 0; }
    int getDefault() { return 0; }
    int total() { return int(tiny); }
}

IConfig makeConfig(int id)
{
    if (id == 0)
    {
        FullConfig c;
        c.tiny = TinyEnum.High;
        c.small = SmallEnum.Full;
        c.def = DefaultEnum.Gamma;
        return c;
    }
    else
    {
        MinimalConfig c;
        c.tiny = TinyEnum.Low;
        return c;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // FullConfig: tiny=High(2), small=Full(200), def=Gamma(3000)
    var a = makeConfig(id);
    outputBuffer[0] = a.getTiny();
    // CHECK: 2
    outputBuffer[1] = a.getSmall();
    // CHECK: 200
    outputBuffer[2] = a.getDefault();
    // CHECK: 3000
    outputBuffer[3] = a.total();
    // CHECK: 3202

    // MinimalConfig: tiny=Low(1)
    var b = makeConfig(id + 1);
    outputBuffer[4] = b.getTiny();
    // CHECK: 1
    outputBuffer[5] = b.getSmall();
    // CHECK: 0
    outputBuffer[6] = b.total();
    // CHECK: 1
}
