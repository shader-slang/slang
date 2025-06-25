//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

// Test that we do not error (should be implicitly inferring capabilities)
// CHECK-NOT: error
// CHECK-NOT: error

public interface IAtomicAddable
{
    public static void atomicAdd(RWByteAddressBuffer buf, uint addr, This value);
}

public extension uint : IAtomicAddable
{
    public static void atomicAdd(RWByteAddressBuffer buf, uint addr, uint value) { buf.InterlockedAdd(addr, value); }
}

public extension int64_t : IAtomicAddable
{
    public static void atomicAdd(RWByteAddressBuffer buf, uint addr, int64_t value) { buf.InterlockedAddI64(addr, value); }
}

RWByteAddressBuffer buffer;
RWStructuredBuffer<uint> data;
[numthreads(1,1,1)]
void computeMain()
{

    data[0].atomicAdd(buffer, 1, 100);
}
