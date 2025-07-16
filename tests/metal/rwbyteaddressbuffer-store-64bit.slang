//TEST:SIMPLE(filecheck=CHECK): -target metal

// Test that RWByteAddressBuffer.Store() with 64-bit values generates valid Metal code
// This is a regression test for https://github.com/shader-slang/slang/issues/7534

RWByteAddressBuffer buffer;

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // 64-bit literals should be truncated to 32-bit before as_type cast
    int64_t i64 = -123;
    buffer.Store(0, i64);
    buffer.Store(0, -123ll);

    uint64_t u64 = 123;
    buffer.Store(0, u64);
    buffer.Store(0, 123ull);
}

// CHECK: as_type<uint32_t>(uint32_t(-123LL))
// CHECK: as_type<uint32_t>(uint32_t(123ULL))