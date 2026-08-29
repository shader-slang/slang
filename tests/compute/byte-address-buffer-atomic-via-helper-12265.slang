// https://github.com/shader-slang/slang/issues/12265
// An atomic on a `RWByteAddressBuffer` passed into a helper must not change the element indexing of
// ordinary `Load`/`Store` on that same buffer: byte offset 12 must read element 3 (value 103).

// CUDA keeps a native byte-address buffer (translateToStructuredBufferOps == false) and exercises
// the fixed path at runtime; Vulkan translates to structured buffers and was already correct.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

// The C++/CPU target cannot run `InterlockedAdd` at runtime (it is capability-gated off the CPU
// target), so lock the emitted C++ shape statically instead: the global buffer must stay a
// `RWByteAddressBuffer` (not be flipped to `RWStructuredBuffer`), and the atomic must take a local
// `.asStructuredBuffer<...>()` view. `-ignore-capabilities` lets the atomic lower for inspection.
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute -ignore-capabilities

//TEST_INPUT:ubuffer(data=[100 101 102 103 104 105 106 107], stride=4):name=inputBuffer
RWByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Passing `buf` by value forces the byte-address-buffer legalization to take its `IRParam`
// path for the atomic; that path must not disturb the byte-address `Load` above.
void incrementAtomic(RWByteAddressBuffer buf, uint offset, uint value)
{
    uint previous;
    buf.InterlockedAdd(offset, value, previous);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    uint loaded = inputBuffer.Load(12);
    incrementAtomic(inputBuffer, 0, 1u);
    outputBuffer[0] = loaded;
}

// CHECK: 103

// NVVM: .visible .entry computeMain
// NVVM: ld.global.u32
// NVVM: atom.global.add.u32
// NVVM: st.global.u32

// The global stays a byte-address buffer and the atomic uses a local structured-buffer view.
// CPP: RWByteAddressBuffer {{[A-Za-z0-9_]+}};
// CPP: asStructuredBuffer<
