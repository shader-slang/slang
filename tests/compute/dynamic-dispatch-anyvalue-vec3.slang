//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -shaderobj -xslang -force-glsl-scalar-layout

// The offsets used by the generated AnyValue packing and
// unpacking functions must match the natural layout of the packed type
// exactly, and the AnyValue size computed for an implementation type must
// match those offsets.
//
// The shape below used to break: a `float3` leaves the packing cursor at a
// 4-mod-8 byte offset, and the old sequential packing walk then placed the
// following struct (which natural layout aligns to 8 bytes for its
// `uint64_t` member) at compacted offsets, so every packed field of
// `Profile` disagreed with its natural-layout offset. The `reinterpret`
// check observes the packed bytes directly and fails in that case. The
// `anyValueSize` is an exact natural-layout fit for `Impl`, so a packing
// walk that needs more bytes than natural layout would silently drop the
// trailing fields and fail the dynamic-dispatch checks as well.
//
// The packing offsets are defined by the *natural* layout of the packed
// type and must not change when the target's buffer layout rules change,
// so the same checks must also hold with `-force-glsl-scalar-layout`
// (last vk test line).

[anyValueSize(40)]
interface IInterface
{
    uint Compute(int which);
};

struct Profile
{
    float factor;    // natural offset 16 inside Impl
    uint64_t handle; // natural offset 24 (8-byte aligned)
    uint width;      // natural offset 32
    uint height;     // natural offset 36
};

struct Impl : IInterface
{
    float3 color;    // natural offset 0
    Profile profile; // natural offset 16: aligned to 8 for `handle`

    uint Compute(int which)
    {
        switch (which)
        {
        case 0: return uint(color.x);
        case 1: return uint(profile.factor);
        case 2: return uint(profile.handle);
        case 3: return uint(profile.handle >> 32);
        case 4: return profile.width;
        default: return profile.height;
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
    obj.profile.width = 7777;
    obj.profile.height = 8888;

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
    outputBuffer[9] = words.w[8];  // width at byte 32
    outputBuffer[10] = words.w[9]; // height at byte 36
    // CHECK: 1082130432
    // CHECK: 572662306
    // CHECK: 286331153
    // CHECK: 7777
    // CHECK: 8888
}
