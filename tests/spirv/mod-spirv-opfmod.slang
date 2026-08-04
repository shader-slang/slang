// Regression test for #12046: the core-module `mod()` function (a floor-modulus,
// `x - y*floor(x/y)`) had no SPIR-V case and fell back to the multi-instruction
// arithmetic expansion. SPIR-V's `OpFMod` is exactly a floor-modulus (its sign
// follows Operand 2), so `mod()` should lower to a single `OpFMod` instruction
// on the direct SPIR-V path. `mod()` is a GLSL-compatibility intrinsic, so the
// test is compiled with `-allow-glsl`. This is a GPU-free check of the emitted
// SPIR-V assembly, covering all three `mod()` overloads.

//TEST:SIMPLE(filecheck=SCALAR): -allow-glsl -stage compute -entry scalarMain -target spirv-asm -emit-spirv-directly
//TEST:SIMPLE(filecheck=VECVEC): -allow-glsl -stage compute -entry vecVecMain -target spirv-asm -emit-spirv-directly
//TEST:SIMPLE(filecheck=VECSCALAR): -allow-glsl -stage compute -entry vecScalarMain -target spirv-asm -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=16):out,name=outputBufferVec
RWStructuredBuffer<float3> outputBufferVec;

// scalar `mod(T, T)` must lower to a single `OpFMod`, not the arithmetic
// expansion (`OpFDiv`/`OpFloor`/`OpFMul`/`OpFSub`).
[numthreads(1, 1, 1)]
void scalarMain()
{
    //SCALAR: OpFMod
    //SCALAR-NOT: OpFloor
    outputBuffer[0] = mod(outputBuffer[0], outputBuffer[1]);
}

// vector `mod(vector<T,N>, vector<T,N>)` lowers to a single `OpFMod`.
[numthreads(1, 1, 1)]
void vecVecMain()
{
    //VECVEC: OpFMod
    //VECVEC-NOT: OpFloor
    outputBufferVec[0] = mod(outputBufferVec[0], outputBufferVec[1]);
}

// vector/scalar `mod(vector<T,N>, T)` broadcasts the divisor
// (`OpCompositeConstruct`) and defers to the vector/vector overload, so it
// too emits `OpFMod`.
[numthreads(1, 1, 1)]
void vecScalarMain()
{
    //VECSCALAR: OpFMod
    //VECSCALAR-NOT: OpFloor
    outputBufferVec[0] = mod(outputBufferVec[0], outputBuffer[0]);
}
