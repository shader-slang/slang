//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry scalarMain
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry halfWordOffsetMain
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry unknownOffsetMain

// Only a `half2` or `half4` at an offset known to be a multiple of 4 is stored as whole words on
// WGSL (see byte-address-buffer-store-half-pairs.slang). These stores would need a
// read-modify-write of a word that another invocation may be writing, so they are still
// rejected.

// CHECK: error[E

RWByteAddressBuffer pool;

[numthreads(1, 1, 1)]
void scalarMain(uint3 tid: SV_DispatchThreadID)
{
    pool.Store<half>(0, half(tid.x));
}

[numthreads(1, 1, 1)]
void halfWordOffsetMain(uint3 tid: SV_DispatchThreadID)
{
    pool.Store<half2>(2, half2(half(tid.x), half(tid.y)));
}

[numthreads(1, 1, 1)]
void unknownOffsetMain(uint3 tid: SV_DispatchThreadID)
{
    // A multiple of 4, but nothing promises it: there is no alignment argument.
    pool.Store<half2>(tid.x * 4, half2(half(tid.x), half(tid.y)));
}
