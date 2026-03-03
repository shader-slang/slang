//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// Test for requirment/implementation conformance given mismatch

public interface IAtomicAddable_Error1
{
    [require(sm_5_0, glsl)]
    [require(sm_5_0, hlsl)]
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int64_t value);
}
public struct AtomicAddable_Error1 : IAtomicAddable_Error1
{
    [require(sm_5_0, glsl)]
    // CHECK: ([[# @LINE+1]]): error 36119: {{.*}}hlsl
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int64_t value) { }
}

// ([[# @LINE-9]]): note: see declaration of 'atomicAdd'

public interface IAtomicAddable_Error2
{
    [require(sm_5_0, glsl)]
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int64_t value);
}
public struct AtomicAddable_Error2 : IAtomicAddable_Error2
{
    [require(sm_5_0, glsl)]
    [require(sm_5_0, hlsl)]
    // CHECK: ([[# @LINE+1]]): error 36108: {{.*}}hlsl
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int64_t value) { }
}

// ([[# @LINE-9]]): note: see declaration of 'atomicAdd'

// Test that we do not error (should be implicitly inferring capabilities)
// CHECK-NOT: error

public interface IAtomicAddable3
{
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int value);
}

public struct AtomicAddable_Error4 : IAtomicAddable3
{
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int value) { buf.InterlockedAdd(addr, value); }
}

public struct AtomicAddable_Error5 : IAtomicAddable3
{
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int value) { buf.InterlockedAddI64(addr, value); }
}

[numthreads(1,1,1)]
void computeMain()
{
}
