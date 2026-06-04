//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T17:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-member-access.md#vector-swizzles
//META: doc_section_digest=ab827430dedad798221ccb6a8d05027ec37ed381ec7b70905cec740e9ed7e864
//META: purpose=Vector swizzle via member: `.r`/`.g`/`.b`/`.a` map to indices 0/1/2/3, parallel to `.x`/`.y`/`.z`/`.w`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `.r`/`.g`/`.b`/`.a` swizzle characters map to vector elements
// 0/1/2/3. An int4 read from a non-constant input buffer is swizzled via .r/.g/.b/.a
// and written to an output buffer; non-constant input defeats constant folding so
// the swizzle survives in emitted code. Each SIMPLE directive checks the index
// sequence (0→r, 1→g, 2→b, 3→a) in the target's emitted text, covering all
// text-emit back-ends including Metal and WGSL.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=VHLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=VGLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=VSPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=VMTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=VWGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=VCUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=VCPP):-target cpp -entry computeMain -stage compute

//TEST_INPUT: set inputBuffer = ubuffer(data=[11 22 33 44], stride=4)
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int4 v = int4(inputBuffer[0], inputBuffer[1], inputBuffer[2], inputBuffer[3]);
    outputBuffer[0] = v.r;
    outputBuffer[1] = v.g;
    outputBuffer[2] = v.b;
    outputBuffer[3] = v.a;
}

// CHECK: type: int32_t
// CHECK-NEXT: 11
// CHECK-NEXT: 22
// CHECK-NEXT: 33
// CHECK-NEXT: 44

// HLSL: .r maps to index 0, .g to 1, .b to 2, .a to 3; the swizzle lowers
// to individual buffer reads. Indices 1..3 load into temporaries _S1.._S3;
// index 0 is read directly. All four loads are present somewhere in the output.
// VHLSL-DAG: inputBuffer_{{[0-9]+}}[int(0)]
// VHLSL-DAG: inputBuffer_{{[0-9]+}}[int(1)]
// VHLSL-DAG: inputBuffer_{{[0-9]+}}[int(2)]
// VHLSL-DAG: inputBuffer_{{[0-9]+}}[int(3)]

// VGLSL-DAG: inputBuffer_{{[0-9]+}}._data[uint(0)]
// VGLSL-DAG: inputBuffer_{{[0-9]+}}._data[uint(1)]
// VGLSL-DAG: inputBuffer_{{[0-9]+}}._data[uint(2)]
// VGLSL-DAG: inputBuffer_{{[0-9]+}}._data[uint(3)]

// SPIRV: loads from indices 0,1,2,3 of inputBuffer then stores to outputBuffer.
// VSPV-DAG: %int_0 = OpConstant %int 0
// VSPV-DAG: %int_1 = OpConstant %int 1
// VSPV-DAG: %int_2 = OpConstant %int 2
// VSPV-DAG: %int_3 = OpConstant %int 3

// Metal: swizzle indices 0..3 appear as +int(0)..+int(3) pointer arithmetic.
// VMTL-DAG: inputBuffer_{{[0-9]+}}+int(0)
// VMTL-DAG: inputBuffer_{{[0-9]+}}+int(1)
// VMTL-DAG: inputBuffer_{{[0-9]+}}+int(2)
// VMTL-DAG: inputBuffer_{{[0-9]+}}+int(3)

// WGSL: swizzle indices 0..3 appear as i32(0)..i32(3) array subscripts.
// VWGSL-DAG: inputBuffer_{{[0-9]+}}[i32(0)]
// VWGSL-DAG: inputBuffer_{{[0-9]+}}[i32(1)]
// VWGSL-DAG: inputBuffer_{{[0-9]+}}[i32(2)]
// VWGSL-DAG: inputBuffer_{{[0-9]+}}[i32(3)]

// CUDA: swizzle indices 0..3 appear as int(0)..int(3) subscripts.
// VCUDA-DAG: inputBuffer_{{[0-9]+}})[int(0)]
// VCUDA-DAG: inputBuffer_{{[0-9]+}})[int(1)]
// VCUDA-DAG: inputBuffer_{{[0-9]+}})[int(2)]
// VCUDA-DAG: inputBuffer_{{[0-9]+}})[int(3)]

// CPP: swizzle indices 0..3 appear as int(0)..int(3) subscripts.
// VCPP-DAG: inputBuffer_{{[0-9]+}})[int(0)]
// VCPP-DAG: inputBuffer_{{[0-9]+}})[int(1)]
// VCPP-DAG: inputBuffer_{{[0-9]+}})[int(2)]
// VCPP-DAG: inputBuffer_{{[0-9]+}})[int(3)]
