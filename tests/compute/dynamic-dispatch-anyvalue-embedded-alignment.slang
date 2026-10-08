//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -shaderobj -xslang -force-glsl-scalar-layout

// AnyValue payloads must be packed at the *natural* layout offsets of the
// packed type: host code, `reinterpret`, and `getAnyValueSize` all assume
// them.
//
// This shape used to break that: the packing walk did not align struct
// starts, so `Profile` was packed at byte 12 instead of its natural
// offset 16 (8-aligned for `handle`), shifting every `Profile` field and
// reading back garbage. (Declaring `profile` before `color` made the
// layouts coincide and hid the bug.)
//
// The `reinterpret` block fails if any field is packed off its natural
// offset; the exact-fit `anyValueSize(40)` fails if the size side
// disagrees with the packing walk.

[anyValueSize(40)]
interface IInterface
{
    uint Compute(int which);
};

struct Profile
{
    float factor;    // natural offset 16 inside Impl
    uint64_t handle; // natural offset 24 (8-byte aligned)
    uint value;      // natural offset 32
};

struct Impl : IInterface
{
    float3 color;    // natural offset 0
    Profile profile; // natural offset 16: aligned to 8 for `handle`
    uint extra;      // natural offset 36

    uint Compute(int which)
    {
        switch (which)
        {
        case 0: return uint(color.x);
        case 1: return uint(profile.factor);
        case 2: return uint(profile.handle);
        case 3: return uint(profile.handle >> 32);
        case 4: return profile.value;
        default: return extra;
        }
    }
};

uint GenericCompute<T : IInterface>(T obj, int which)
{
    return obj.Compute(which);
}

// One uint per 4 bytes of `Impl`'s natural layout.
struct Words
{
    uint w[10];
};

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Impl obj;
    obj.color = float3(10.0, 20.0, 30.0);
    obj.profile.factor = 4.0;
    obj.profile.handle = 0x1111111122222222ULL;
    obj.profile.value = 7777;
    obj.extra = 8888;

    // Dynamic dispatch must round-trip every field through the exact-fit
    // AnyValue payload.
    for (int i = 0; i < 6; i++)
        outputBuffer[i] = GenericCompute<Impl>(obj, i);
    // CHECK: 10
    // CHECK: 4
    // CHECK: 572662306
    // CHECK: 286331153
    // CHECK: 7777
    // CHECK: 8888

    // `reinterpret` packs into an AnyValue and unpacks the raw words, so it
    // observes the packed byte offsets directly: each field must sit at its
    // natural-layout offset.
    Words words = reinterpret<Words>(obj);
    outputBuffer[6] = words.w[4];  // factor at byte 16
    outputBuffer[7] = words.w[6];  // handle low word at byte 24
    outputBuffer[8] = words.w[7];  // handle high word at byte 28
    outputBuffer[9] = words.w[8];  // value at byte 32
    outputBuffer[10] = words.w[9]; // extra at byte 36
    // CHECK: 1082130432
    // CHECK: 572662306
    // CHECK: 286331153
    // CHECK: 7777
    // CHECK: 8888
}
