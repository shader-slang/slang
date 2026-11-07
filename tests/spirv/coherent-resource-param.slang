//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry main

globallycoherent RWByteAddressBuffer gBuffer;

void storeValue(globallycoherent RWByteAddressBuffer buffer, uint value)
{
    buffer.Store(0, value);
}

[numthreads(1, 1, 1)]
void main()
{
    storeValue(gBuffer, 1);
}

// The program must compile without the front-end rejecting the parameter, and
// the coherence must survive to emission on both targets.

// On SPIR-V the helper is specialized onto the concrete global, which carries
// the coherent decoration.
// SPIRV: OpDecorate %gBuffer Coherent

// On HLSL the helper is emitted as a separate function, so the parameter itself
// must carry `globallycoherent` for the callee's store to remain coherent.
// HLSL: void storeValue{{.*}}(globallycoherent RWByteAddressBuffer
