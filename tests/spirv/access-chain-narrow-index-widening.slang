// Regression for shader-slang/slang#13194. SPIR-V interprets every OpAccessChain /
// OpPtrAccessChain index operand as *signed* regardless of the index type's declared
// signedness (per the SPIR-V spec's access-chain indexing semantics), so a sub-32-bit unsigned index such as a uint16_t
// of 40000 was read as the negative offset -25536 and addressed out of bounds. Sub-32-bit
// indices must therefore be widened to 32 bits (same signedness) before the access chain.
//
// Each case is its own entry point so its checks are isolated, and each index is derived from
// SV_DispatchThreadID (dynamic, not constant-folded) so the widening appears as an explicit
// convert whose result id is threaded into the access chain -- i.e. the checks are non-vacuous.

//TEST:SIMPLE(filecheck=CHECKU): -target spirv-asm -emit-spirv-directly -entry mainU -stage compute
//TEST:SIMPLE(filecheck=CHECKU8): -target spirv-asm -emit-spirv-directly -entry mainU8 -stage compute
//TEST:SIMPLE(filecheck=CHECKS): -target spirv-asm -emit-spirv-directly -entry mainS -stage compute
//TEST:SIMPLE(filecheck=CHECKW): -target spirv-asm -emit-spirv-directly -entry mainW -stage compute
//TEST:SIMPLE(filecheck=CHECKB): -target spirv-asm -emit-spirv-directly -entry mainB -stage compute
//TEST:SIMPLE(filecheck=CHECKR): -target spirv-asm -emit-spirv-directly -entry mainR -stage compute
//TEST:SIMPLE(filecheck=CHECKP): -target spirv-asm -emit-spirv-directly -entry mainP -stage compute
//TEST:SIMPLE(filecheck=CHECKN): -target spirv-asm -emit-spirv-directly -entry mainN -stage compute

groupshared float arr[50000];
RWStructuredBuffer<float> outBuf;

// uint16 index: zero-extended (OpUConvert to %uint); the [[U]] binding ties the widened id to
// the access-chain operand so the check fails if the narrow index is forwarded instead.
// CHECKU: %[[U:[0-9]+]] = OpUConvert %uint %{{[^ ]+}}
// CHECKU: OpAccessChain %{{[^ ]+}} %arr %[[U]]
[shader("compute")]
[numthreads(1, 1, 1)]
void mainU(uint3 tid : SV_DispatchThreadID)
{
    uint16_t u = (uint16_t)tid.x;
    outBuf[0] = arr[u];
}

// uint8 index: same widening as uint16 (the bug applies to uint8_t >= 128 too).
// CHECKU8: %[[U8:[0-9]+]] = OpUConvert %uint %{{[^ ]+}}
// CHECKU8: OpAccessChain %{{[^ ]+}} %arr %[[U8]]
[shader("compute")]
[numthreads(1, 1, 1)]
void mainU8(uint3 tid : SV_DispatchThreadID)
{
    uint8_t u = (uint8_t)tid.x;
    outBuf[0] = arr[u];
}

// int16 index: sign-extended (OpSConvert to %int, so the result type distinguishes it from the
// incidental down-cast to %short).
// CHECKS: %[[S:[0-9]+]] = OpSConvert %int %{{[^ ]+}}
// CHECKS: OpAccessChain %{{[^ ]+}} %arr %[[S]]
[shader("compute")]
[numthreads(1, 1, 1)]
void mainS(uint3 tid : SV_DispatchThreadID)
{
    int16_t s = (int16_t)tid.x;
    outBuf[0] = arr[s];
}

// 32-bit index: the >= 32-bit boundary that must NOT be widened (no spurious convert), which is
// what keeps the wider 64-bit indexing path untouched (that path itself is covered by
// tests/spirv/shader-64bit-indexing-functional.slang).
// CHECKW-NOT: OpUConvert
// CHECKW-NOT: OpSConvert
// CHECKW: OpAccessChain %{{[^ ]+}} %arr %{{[^ ]+}}
[shader("compute")]
[numthreads(1, 1, 1)]
void mainW(uint3 tid : SV_DispatchThreadID)
{
    uint w = tid.x;
    outBuf[0] = arr[w];
}

// RWStructuredBuffer subscript: the index is the last access-chain operand (after the %int_0
// element selector). The store lowers to a RWStructuredBufferGetElementPtr created mid-pass.
// CHECKB: %[[B:[0-9]+]] = OpUConvert %uint %{{[^ ]+}}
// CHECKB: OpAccessChain %{{[^ ]+}} %{{[^ ]+}} %int_0 %[[B]]
[shader("compute")]
[numthreads(1, 1, 1)]
void mainB(uint3 tid : SV_DispatchThreadID)
{
    uint16_t u = (uint16_t)tid.x;
    outBuf[u] = 1.0f;
}

StructuredBuffer<float> inBuf;

// Read-only StructuredBuffer load: a distinct producer (StructuredBufferLoad, lowered to a
// RWStructuredBufferGetElementPtr) whose generic subscript also preserves a narrow index.
// CHECKR: %[[R:[0-9]+]] = OpUConvert %uint %{{[^ ]+}}
// CHECKR: OpAccessChain %{{[^ ]+}} %{{[^ ]+}} %int_0 %[[R]]
[shader("compute")]
[numthreads(1, 1, 1)]
void mainR(uint3 tid : SV_DispatchThreadID)
{
    uint16_t i = (uint16_t)tid.x;
    outBuf[0] = inBuf[i];
}

// PhysicalStorageBuffer pointer offset: the GetOffsetPtr producer, emitted as OpPtrAccessChain
// (whose offset operand is likewise signed-interpreted), so a narrow offset must be widened too.
// CHECKP: %[[P:[0-9]+]] = OpUConvert %uint %{{[^ ]+}}
// CHECKP: OpPtrAccessChain %{{[^ ]+}} %{{[^ ]+}} %[[P]]
[shader("compute")]
[numthreads(1, 1, 1)]
void mainP(uint3 tid : SV_DispatchThreadID, uniform float* p)
{
    uint16_t i = (uint16_t)tid.x;
    p[i] = 1.0f;
}

RWStructuredBuffer<float> buffers[];

// A narrow *non-uniform* (bindless) descriptor index must keep its NonUniform marker across the
// widening cast: propagateNonUniformDecorations decorates the index operand but does not look
// through the inserted cast, so the widened value the access chain consumes has to stay decorated
// (the resulting pointer's NonUniform requirement, Vulkan VUID-RuntimeSpirv-None-10148, depends on
// it). The CHECK-DAG triple binds the widened id to its decoration and to the access-chain operand.
// CHECKN-DAG: %[[N:[0-9]+]] = OpUConvert %uint %{{[^ ]+}}
// CHECKN-DAG: OpDecorate %[[N]] NonUniform
// CHECKN-DAG: OpAccessChain %{{[^ ]+}} %{{[^ ]+}} %[[N]]
[shader("compute")]
[numthreads(1, 1, 1)]
void mainN(uint3 tid : SV_DispatchThreadID)
{
    uint16_t i = (uint16_t)tid.x;
    buffers[NonUniformResourceIndex(i)][0] = 1.0f;
}
