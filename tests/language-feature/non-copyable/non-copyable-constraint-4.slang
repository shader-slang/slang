// input into struct copyable type => T is untagged so it only allows copyable types ==> func param is untagged so it only allows copyable types.
//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv

// input into struct copyable type => T is tagged noncopyable so it allows noncopyable and copyable types ==> [FAIL] func param is untagged so it only allows copyable types.
//TEST(compute):SIMPLE(filecheck=CHECK_FAIL): -stage compute -entry computeMain -target spirv -DWITH_CONSTRAINT_ON_TYPE

// input into struct noncopyable type => [FAIL] T is untagged so it only allows copyable types.
//TEST(compute):SIMPLE(filecheck=CHECK_FAIL): -stage compute -entry computeMain -target spirv -DNON_COPYABLE_PARAM

// input into struct copyable type => T is untagged so it only allows copyable types ==> func param is tagged noncopyable so it allows non-copyable & copyable types.
//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv -DWITH_CONSTRAINT_ON_FUNC

// input into struct noncopyable type => T is tagged noncopyable so it allows noncopyable and copyable types ==> [FAIL] func param is untagged so it only allows copyable types.
//TEST(compute):SIMPLE(filecheck=CHECK_FAIL): -stage compute -entry computeMain -target spirv -DWITH_CONSTRAINT_ON_TYPE -DNON_COPYABLE_PARAM

// input into struct copyable type => T is tagged noncopyable so it allows noncopyable and copyable types ==> func param is tagged noncopyable so it allows non-copyable & copyable types.
//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv -DWITH_CONSTRAINT_ON_TYPE -DWITH_CONSTRAINT_ON_FUNC

// input into struct noncopyable type => [FAIL] T is untagged so it only allows copyable types
//TEST(compute):SIMPLE(filecheck=CHECK_FAIL): -stage compute -entry computeMain -target spirv -DWITH_CONSTRAINT_ON_FUNC -DNON_COPYABLE_PARAM

// input into struct noncopyable type => T is tagged noncopyable so it allows noncopyable and copyable types ==> func param is tagged noncopyable so it allows non-copyable & copyable types.
//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv -DWITH_CONSTRAINT_ON_TYPE -DWITH_CONSTRAINT_ON_FUNC -DNON_COPYABLE_PARAM

interface IFoo
{
    __init(int v);
}

[__NonCopyableType]
struct NonCopyableType : IFoo
{
    __init(int v) { x = v; }
    int x;
}

struct CopyableType : IFoo
{
    __init(int v) { x = v; }
    int x;
}

// CHECK_PASS: OpEntryPoint
// CHECK_PASS-NOT: error
// CHECK_FAIL: error 38042
struct MaybeNonCopyableType<T>
#ifdef WITH_CONSTRAINT_ON_TYPE
    where noncopyable(T)
#endif
{
    T inner;
}

void foo<T>(T t)
#ifdef WITH_CONSTRAINT_ON_FUNC
    where noncopyable(T)
#endif
{
};

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
#ifdef NON_COPYABLE_PARAM
    MaybeNonCopyableType<NonCopyableType> val;
    foo<MaybeNonCopyableType<NonCopyableType>>(val);
#else
    MaybeNonCopyableType<CopyableType> val;
    foo<MaybeNonCopyableType<CopyableType>>(val);
#endif
}
