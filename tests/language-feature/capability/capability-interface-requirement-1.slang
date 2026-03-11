//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

// Test for requirment/implementation conformance given mismatch

public interface IAtomicAddable_Error1
{
    [require(sm_5_0, glsl)]
    [require(sm_5_0, hlsl)]
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int64_t value);
//CHECK:        ^^^^^^^^^ see declaration of 'atomicAdd'
}
public struct AtomicAddable_Error1 : IAtomicAddable_Error1
{
    [require(sm_5_0, glsl)]
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int64_t value) { }
//CHECK:        ^^^^^^^^^ requirement missing target/stage support
//CHECK:        ^^^^^^^^^ requirement 'atomicAdd' must have the same target/stage support as the implementation; 'atomicAdd' is missing 'hlsl'
}

public interface IAtomicAddable_Error2
{
    [require(sm_5_0, glsl)]
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int64_t value);
//CHECK:        ^^^^^^^^^ see declaration of 'atomicAdd'
}
public struct AtomicAddable_Error2 : IAtomicAddable_Error2
{
    [require(sm_5_0, glsl)]
    [require(sm_5_0, hlsl)]
    public void atomicAdd(RWByteAddressBuffer buf, uint addr, int64_t value) { }
//CHECK:        ^^^^^^^^^ dependencies not compatible on target
//CHECK:        ^^^^^^^^^ 'atomicAdd' has dependencies that are not compatible on the required compilation target 'hlsl'.
}

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
