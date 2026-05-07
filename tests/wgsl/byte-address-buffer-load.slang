// Regression for #11019.  Read-only `ByteAddressBuffer::Load*` overloads
// were missing `wgsl` from their capability `[require(...)]` annotations
// and failed to compile on the WGSL target with E36107, while the
// `RWByteAddressBuffer` counterparts worked fine.  This test exercises
// `Load`, `Load2`, `Load3`, and `Load4` to confirm the asymmetry is
// gone.  A compile-only SIMPLE test is sufficient; we just need the
// frontend to accept the overloads for the WGSL target.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry main -stage compute

struct Buffer
{
    ByteAddressBuffer ba;
    RWByteAddressBuffer rw;
};

ParameterBlock<Buffer> buffers;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint  a = buffers.ba.Load(0);
    uint2 b = buffers.ba.Load2(4);
    uint3 c = buffers.ba.Load3(8);
    uint4 d = buffers.ba.Load4(16);

    // CHECK: fn main
    buffers.rw.Store(0, a + b.x + c.x + d.x);
}
