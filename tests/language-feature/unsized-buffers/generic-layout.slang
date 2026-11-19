//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -validate-ir -O0 -matrix-layout-row-major
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -validate-ir -O0 -matrix-layout-row-major
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -validate-ir -O0 -matrix-layout-row-major
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -validate-ir -O0 -matrix-layout-row-major
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -validate-ir -O0 -matrix-layout-row-major
//TEST:SIMPLE(filecheck=SPIRV): -target wgsl-spirv-asm -entry computeMain -stage compute -validate-ir -matrix-layout-row-major
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -validate-ir -O0 -matrix-layout-row-major -DSKIP_DIMENSIONS=1
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute -validate-ir -matrix-layout-row-major -DSKIP_DIMENSIONS=1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cpu -compute -output-using-type -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cuda -compute -output-using-type -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -llvm -compute -output-using-type -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=RESULT): -vk -compute -output-using-type -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -wgpu -compute -output-using-type -xslang -matrix-layout-row-major
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=METAL-RESULT): -metal -compute -output-using-type -xslang -matrix-layout-row-major -DSKIP_DIMENSIONS=1

// Element and layout types pass through generic specialization and interface witnesses.
// Returning views through noinline helpers also exercises the prelude struct call ABI.
// The matrix element makes HLSL reconstruct its buffer type to preserve row-major storage.
// float3x2 has the same 24-byte stride under scalar layout and WGSL's native matrix layout.
interface IBufferViews
{
    associatedtype Element;
    associatedtype Layout : IBufferDataLayout;

    __UnsizedStructuredBuffer<Element, Layout> getReadView();
    __UnsizedRWStructuredBuffer<Element, Layout> getWriteView();
}

[noinline]
__UnsizedStructuredBuffer<T, L> makeReadView<T, L : IBufferDataLayout>(
    StructuredBuffer<T, L> buffer)
{
    return buffer;
}

[noinline]
__UnsizedRWStructuredBuffer<T, L> makeWriteView<T, L : IBufferDataLayout>(
    RWStructuredBuffer<T, L> buffer)
{
    return __UnsizedRWStructuredBuffer<T, L>(buffer);
}

struct BufferViews<T, L : IBufferDataLayout> : IBufferViews
{
    typealias Element = T;
    typealias Layout = L;

    StructuredBuffer<T, L> input;
    RWStructuredBuffer<T, L> output;

    __UnsizedStructuredBuffer<T, L> getReadView() { return makeReadView(input); }
    __UnsizedRWStructuredBuffer<T, L> getWriteView() { return makeWriteView(output); }
}

[noinline]
V.Element copyThroughWitness<V : IBufferViews>(V views, uint index)
{
    let value = views.getReadView()[index];
    views.getWriteView()[index] = value;
    return value;
}

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0], stride=24):name=input
StructuredBuffer<float3x2, ScalarDataLayout> input;
//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0], stride=24):name=copied
RWStructuredBuffer<float3x2, ScalarDataLayout> copied;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    BufferViews<float3x2, ScalarDataLayout> views;
    views.input = input;
    views.output = copied;
    let value = copyThroughWitness(views, 1);
    output[0] = value[0][1] == input[1][0][1];
    output[1] = value[1][0] == input[1][1][0];
    output[2] = copied[1][2][1] == input[1][2][1];
#ifndef SKIP_DIMENSIONS
    // Sized buffers retain their dimension API on targets that support it. Metal does not.
    uint count, stride;
    views.input.GetDimensions(count, stride);
    output[3] = count;
#endif
}

// CUDA-DAG: UnsizedStructuredBuffer<{{.*}}> makeReadView
// CUDA-DAG: UnsizedRWStructuredBuffer<{{.*}}> makeWriteView
// CPP-DAG: UnsizedStructuredBuffer<{{.*}}> makeReadView
// CPP-DAG: UnsizedRWStructuredBuffer<{{.*}}> makeWriteView
// HLSL-DAG: StructuredBuffer<
// HLSL-DAG: RWStructuredBuffer<
// HLSL: computeMain
// SPIRV: OpEntryPoint GLCompute
// WGSL-DAG: var<storage, read> input{{[_0-9]*}} : array<mat3x2<f32>>;
// WGSL-DAG: var<storage, read_write> copied{{[_0-9]*}} : array<mat3x2<f32>>;
// WGSL: fn copyThroughWitness
// WGSL: copied{{[_0-9]*}}[{{.*}}] =
// WGSL: arrayLength(&input
// METAL: struct _MatrixStorage_float3x2natural
// METAL: array<packed_float2, int(3)> data
// METAL-DAG: const _MatrixStorage_float3x2natural{{[_0-9]*}} device* input
// METAL-DAG: _MatrixStorage_float3x2natural{{[_0-9]*}} device* copied
// METALLIB: define void @computeMain
// RESULT: 1
// RESULT: 1
// RESULT: 1
// RESULT: 2
// METAL-RESULT: 1
// METAL-RESULT: 1
// METAL-RESULT: 1
// METAL-RESULT: 0
