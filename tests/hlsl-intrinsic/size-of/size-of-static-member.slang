// Static struct members are lowered outside the struct (as a global variable
// or a constant), so they must not contribute to the size or alignment of an
// instance. A concrete `sizeof(S)`/`alignof(S)`
// is folded by the front end, while `sizeof(T)` inside a generic is folded
// later from the lowered IR struct, which never contains static members. Both
// paths must agree. See shader-slang/slang#13858.

#pragma warning(disable:30816)

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -dx12 -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[10 11 12 13 14 15 16 17], stride=4):name records
ByteAddressBuffer records;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):name roundTripRecords
RWByteAddressBuffer roundTripRecords;

// `Record` holds `uint`s so that the round trip below can compare raw input
// words without float conversion.
struct Record
{
    static const uint VERSION = 2;
    uint data[2];
}

struct Leading
{
    static const uint VERSION = 2;
    float data[2];
}

struct Trailing
{
    float data[2];
    static const uint VERSION = 2;
}

struct Mutable
{
    static uint counter;
    float data[2];
}

struct TwoStatics
{
    static const uint A = 1;
    static const uint B = 2;
    float data[2];
}

struct WideStatic
{
    static const uint64_t BIG = 1;
    float data[2];
}

struct Base
{
    static const uint TAG = 7;
    float a;
}

struct Derived : Base
{
    float b;
}

struct Nested
{
    Leading inner;
    float tail;
}

struct G<T>
{
    static const uint K = 3;
    float data[2];
}

// With no instance fields the natural layout is the empty size (0 bytes,
// alignment 1), which is also what IR layout gives the lowered empty struct.
struct OnlyStatic
{
    static const uint X = 1;
}

// A static member of the struct's own type is legal because it does not live
// in an instance. Before the fix the front end gave up on this type and the
// IR fallback produced 4, so only the `SizeTag` check below, which needs a
// front-end constant, detects the change.
struct SelfStatic
{
    static SelfStatic shared;
    float x;
}

// Controls: none of these declare a static data member.
struct StaticMethod
{
    static uint get() { return 1; }
    float data[2];
}

struct InstanceConst
{
    const uint c = 1;
    float data[2];
}

// A generic value argument is compared as a folded AST value, so this only
// type-checks when the front end folds `sizeof(SelfStatic)` to a constant.
struct SizeTag<let N : int>
{
    int get() { return N; }
}

void takesSizeTag4(SizeTag<4> t) {}

uint genericSize<T>()
{
    return sizeof(T);
}

uint genericAlign<T>()
{
    return alignof(T);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    static_assert(sizeof(Leading) == 8, "leading static member counted in sizeof");
    static_assert(sizeof(Trailing) == 8, "trailing static member counted in sizeof");
    static_assert(sizeof(Mutable) == 8, "mutable static member counted in sizeof");
    static_assert(sizeof(TwoStatics) == 8, "two static members counted in sizeof");
    static_assert(sizeof(WideStatic) == 8, "static uint64_t counted in sizeof");
    static_assert(alignof(WideStatic) == 4, "static uint64_t counted in alignof");
    static_assert(sizeof(Derived) == 8, "base static member counted in sizeof");
    static_assert(sizeof(Nested) == 12, "nested static member counted in sizeof");
    static_assert(sizeof(G<int>) == 8, "generic struct static member counted in sizeof");
    static_assert(sizeof(Leading, ScalarDataLayout) == 8, "static member counted in scalar sizeof");
    static_assert(sizeof(StaticMethod) == 8, "static method control");
    static_assert(sizeof(InstanceConst) == 12, "non-static const control");
    static_assert(sizeof(OnlyStatic) == 0, "static-only struct is not empty");
    static_assert(sizeof(Leading, Std430DataLayout) == 8, "std430 control");
    static_assert(alignof(OnlyStatic) == 1, "static-only alignof");
    static_assert(sizeof(Leading[3]) == 24, "array of affected struct");
    static_assert(sizeof(SelfStatic) == 4, "self-typed static member agrees with IR layout");

    SizeTag<sizeof(SelfStatic)> selfTag;
    takesSizeTag4(selfTag);

    int arr[sizeof(Leading)];
    static_assert(countof(arr) == 8, "static member counted in array size");

    // CHECK: type: uint32_t
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    outputBuffer[0] = sizeof(Leading);
    outputBuffer[1] = genericSize<Leading>();
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    outputBuffer[2] = sizeof(Trailing);
    outputBuffer[3] = genericSize<Trailing>();
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    outputBuffer[4] = sizeof(Mutable);
    outputBuffer[5] = genericSize<Mutable>();
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    outputBuffer[6] = sizeof(TwoStatics);
    outputBuffer[7] = genericSize<TwoStatics>();
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    outputBuffer[8] = sizeof(WideStatic);
    outputBuffer[9] = genericSize<WideStatic>();
    // CHECK-NEXT: 4
    // CHECK-NEXT: 4
    outputBuffer[10] = alignof(WideStatic);
    outputBuffer[11] = genericAlign<WideStatic>();
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    outputBuffer[12] = sizeof(Derived);
    outputBuffer[13] = genericSize<Derived>();
    // CHECK-NEXT: 12
    // CHECK-NEXT: 12
    outputBuffer[14] = sizeof(Nested);
    outputBuffer[15] = genericSize<Nested>();
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    outputBuffer[16] = sizeof(G<int>);
    outputBuffer[17] = genericSize<G<int>>();
    // Explicit layouts: only `ScalarDataLayout` is folded by the front end;
    // `Std430DataLayout` is an IR-layout control.
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    outputBuffer[18] = sizeof(Leading, ScalarDataLayout);
    outputBuffer[19] = sizeof(Leading, Std430DataLayout);
    // CHECK-NEXT: 8
    outputBuffer[20] = countof(arr);
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    outputBuffer[21] = sizeof(StaticMethod);
    outputBuffer[22] = genericSize<StaticMethod>();
    // CHECK-NEXT: 12
    // CHECK-NEXT: 12
    outputBuffer[23] = sizeof(InstanceConst);
    outputBuffer[24] = genericSize<InstanceConst>();
    // CHECK-NEXT: 4
    // CHECK-NEXT: 4
    outputBuffer[25] = alignof(Leading);
    outputBuffer[26] = genericAlign<Leading>();
    // CHECK-NEXT: 0
    // CHECK-NEXT: 0
    outputBuffer[27] = sizeof(OnlyStatic);
    outputBuffer[28] = genericSize<OnlyStatic>();
    // CHECK-NEXT: 4
    // CHECK-NEXT: 4
    outputBuffer[29] = sizeof(SelfStatic);
    outputBuffer[30] = genericSize<SelfStatic>();

    // Byte-offset arithmetic with a concrete `sizeof` must address the same
    // records as the buffer layout does. The stores are read back at fixed
    // offsets so that a wrong store stride cannot cancel out.
    // CHECK-NEXT: 10
    // CHECK-NEXT: 11
    // CHECK-NEXT: 12
    // CHECK-NEXT: 13
    // CHECK-NEXT: 14
    // CHECK-NEXT: 15
    for (uint i = 0; i < 3; i++)
    {
        let record = records.Load<Record>(i * sizeof(Record));
        roundTripRecords.Store<Record>(i * sizeof(Record), record);
    }
    for (uint i = 0; i < 6; i++)
    {
        outputBuffer[31 + i] = roundTripRecords.Load(i * 4);
    }
}
