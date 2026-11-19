//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -llvm -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute,vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute,metal):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -compute -output-using-type -xslang -matrix-layout-column-major
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -validate-ir -O0 -matrix-layout-column-major
//TEST:SIMPLE(filecheck=SPIRV): -target wgsl-spirv-asm -entry computeMain -stage compute -validate-ir -matrix-layout-column-major
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -validate-ir -O0 -matrix-layout-column-major
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute -validate-ir -matrix-layout-column-major

// An unsized view must preserve its source buffer's physical matrix layout. Using asymmetric
// input and comparing both off-diagonal elements catches an accidental transpose on conversion.
struct Element
{
    float2x2 matrix;
}

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=16):name=source
StructuredBuffer<Element> source;
//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 0.0], stride=16):name=destination
RWStructuredBuffer<Element> destination;
//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[noinline]
__UnsizedStructuredBuffer<T> makeView<T>(StructuredBuffer<T> buffer)
{
    return buffer;
}

[noinline]
__UnsizedRWStructuredBuffer<T> makeWritableView<T>(RWStructuredBuffer<T> buffer)
{
    return buffer;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    let view = makeView(source);
    output[0] = source[0].matrix[0][1] == view[0].matrix[0][1];
    output[1] = source[0].matrix[1][0] == view[0].matrix[1][0];

    let writableView = makeWritableView(destination);
    writableView[0].matrix[0][1] = 7.0;
    output[2] = destination[0].matrix[0][1] == 7.0;
}

// WGSL: struct _MatrixStorage_float2x2_ColMajorstd430
// WGSL: data{{[_0-9]*}} : array<vec2<f32>, i32(2)>
// WGSL-DAG: var<storage, read> source{{[_0-9]*}} : array<Element_std430
// WGSL-DAG: var<storage, read_write> destination{{[_0-9]*}} : array<Element_std430
// WGSL: destination{{[_0-9]*}}[i32(0)].matrix{{[_0-9]*}}.data{{[_0-9]*}}[i32(1)][i32(0)] = 7.0f;
// SPIRV: OpEntryPoint GLCompute
// METAL: struct _MatrixStorage_float2x2_ColMajornatural
// METAL: array<packed_float2, int(2)> data
// METAL-DAG: const Element_natural{{[_0-9]*}} device* source
// METAL-DAG: Element_natural{{[_0-9]*}} device* destination
// METAL: destination{{[_0-9]*}}+int(0))->matrix{{[_0-9]*}})->data{{[_0-9]*}}[int(1)][int(0)] = 7.0f;
// METALLIB: define void @computeMain

// CHECK: 1
// CHECK: 1
// CHECK: 1
