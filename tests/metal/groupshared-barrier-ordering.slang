// Metal materializes each `groupshared` global as a `threadgroup` local of the entry
// point, reached from other functions through the kernel context. Loads and stores of
// that memory must not be forwarded, re-loaded or eliminated across a barrier or any
// other call, including a side-effect-free call that only reads it.

//TEST:SIMPLE(filecheck=PERMUTE): -target metal -stage compute -entry permuteMain
//TEST:SIMPLE(filecheck=CALL): -target metal -stage compute -entry callMain
//TEST:SIMPLE(filecheck=DSE): -target metal -stage compute -entry deadStoreMain
//TEST:SIMPLE(filecheck=HELPER): -target metal -stage compute -entry helperMain
//TEST:SIMPLE(filecheck=PURE): -target metal -stage compute -entry pureReaderMain

// The post-barrier store must store the value read before the barrier, not re-load it,
// and the final read must come from groupshared memory, not from `v`.
// PERMUTE: float2 [[V:v_[0-9]+]] = [[S:permuteShared_[0-9]+]]{{\[}}63U - [[T:t_[0-9]+]]{{\]}};
// PERMUTE: threadgroup_barrier
// PERMUTE-NOT: {{\[}}63U
// PERMUTE: [[S]]{{\[}}[[T]]{{\]}} = [[V]];
// PERMUTE: threadgroup_barrier
// PERMUTE: packed_float2([[S]]{{\[}}[[T]]{{\]}})

groupshared float2 permuteShared[64];
RWStructuredBuffer<float2> permuteBuf;

[numthreads(64, 1, 1)]
void permuteMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    permuteShared[t] = permuteBuf[t];
    GroupMemoryBarrierWithGroupSync();
    float2 v = permuteShared[63 - t];
    GroupMemoryBarrierWithGroupSync();
    permuteShared[t] = v;
    GroupMemoryBarrierWithGroupSync();
    permuteBuf[t] = permuteShared[t];
}

// The callee overwrites the slot, so the read after the call must not be folded to 1.0.
// CALL: [[S:callShared_[0-9]+]]{{\[}}[[T:t_[0-9]+]]{{\]}} = 1.0f;
// CALL: writeShared_{{[0-9]+}}([[T]], 2.0f,
// CALL: = [[S]]{{\[}}[[T]]{{\]}};

groupshared float callShared[64];
RWStructuredBuffer<float> callBuf;

[noinline]
void writeShared(uint i, float x)
{
    callShared[i] = x;
}

[numthreads(64, 1, 1)]
void callMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    callShared[t] = 1.0;
    writeShared(t, 2.0);
    callBuf[t] = callShared[t];
}

// The neighbour reads the first store between the barriers, so it is not dead.
// DSE: void deadStoreMain(
// DSE: [[S:storeShared_[0-9]+]]{{\[}}[[T:t_[0-9]+]]{{\]}} = 1.0f;
// DSE: threadgroup_barrier
// DSE: readNeighbour_{{[0-9]+}}([[T]],
// DSE: threadgroup_barrier
// DSE: [[S]]{{\[}}[[T]]{{\]}} = 2.0f;

groupshared float storeShared[64];
RWStructuredBuffer<float> storeBuf;

[noinline]
void readNeighbour(uint t)
{
    storeBuf[t] = storeShared[(t + 1) % 64];
}

[numthreads(64, 1, 1)]
void deadStoreMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    storeShared[t] = 1.0;
    GroupMemoryBarrierWithGroupSync();
    readNeighbour(t);
    GroupMemoryBarrierWithGroupSync();
    storeShared[t] = 2.0;
}

// The same permutation inside a helper, where groupshared memory is reached through
// a pointer loaded from the kernel context rather than through a local variable.
// HELPER: float permuteInHelper_{{[0-9]+}}(
// HELPER: float [[V:v_[0-9]+]] = (*[[S:kernelContext_[0-9]+->helperShared_[0-9]+]]){{\[}}63U - [[T:t_[0-9]+]]{{\]}};
// HELPER: threadgroup_barrier
// HELPER-NOT: {{\[}}63U
// HELPER: (*[[S]]){{\[}}[[T]]{{\]}} = [[V]];
// HELPER: threadgroup_barrier
// HELPER: return (*[[S]]){{\[}}[[T]]{{\]}} +

groupshared float helperShared[64];
RWStructuredBuffer<float> helperBuf;

[noinline]
float permuteInHelper(uint t)
{
    float v = helperShared[63 - t];
    GroupMemoryBarrierWithGroupSync();
    helperShared[t] = v;
    GroupMemoryBarrierWithGroupSync();
    return helperShared[t] + helperShared[(t + 1) % 64];
}

[numthreads(64, 1, 1)]
void helperMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    helperShared[t] = helperBuf[t];
    GroupMemoryBarrierWithGroupSync();
    helperBuf[t] = permuteInHelper(t);
}

// A callee without side effects still reads groupshared memory, so the store it
// observes is not dead.
// PURE: void pureReaderMain(
// PURE: [[S:pureShared_[0-9]+]]{{\[}}[[T:t_[0-9]+]]{{\]}} = 1.0f;
// PURE: readShared_{{[0-9]+}}([[T]],
// PURE: [[S]]{{\[}}[[T]]{{\]}} = 2.0f;

groupshared float pureShared[64];
RWStructuredBuffer<float> pureBuf;

[noinline]
float readShared(uint t)
{
    return pureShared[t];
}

[numthreads(64, 1, 1)]
void pureReaderMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    pureShared[t] = 1.0;
    float x = readShared(t);
    pureShared[t] = 2.0;
    pureBuf[t] = x + pureShared[t];
}
