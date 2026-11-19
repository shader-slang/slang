//TEST:SIMPLE(filecheck=COMPILE): -target cuda -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=COMPILE): -target cpp -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=COMPILE): -target hlsl -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -validate-ir -DUINT_FLAGS=1
//TEST:SIMPLE(filecheck=SPIRV): -target wgsl-spirv-asm -entry computeMain -stage compute -validate-ir -DUINT_FLAGS=1
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=COMPILE): -target llvm-shader-ir -entry computeMain -stage compute -validate-ir
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -wgpu -compute -output-using-type -xslang -DUINT_FLAGS=1
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=RESULT): -metal -compute -output-using-type

// Matrix and Boolean elements exercise buffer element legalization before a sized view is
// converted to an unsized view. The resulting views must still agree about the element layout.
// WGSL storage buffers cannot contain bool, so use uint for its flag representation.
#ifdef UINT_FLAGS
typealias Flag = uint;
#else
typealias Flag = bool;
#endif

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0], stride=24):name=matrices
StructuredBuffer<float2x3> matrices;
//TEST_INPUT:ubuffer(data=[0], stride=4):name=flags
RWStructuredBuffer<Flag> flags;
//TEST_INPUT:ubuffer(data=[0.0], stride=4):out,name=output
RWStructuredBuffer<float> output;

struct MatrixViews
{
    StructuredBuffer<float2x3> sized;
    __UnsizedStructuredBuffer<float2x3> unsized;
}

void update(MatrixViews views, __UnsizedRWStructuredBuffer<Flag> writableFlags,
    __UnsizedRWStructuredBuffer<float> writableOutput)
{
    writableFlags[0] = Flag(views.sized[0][0][0] > 0.0);
    writableOutput[0] = bool(writableFlags[0]) ? views.unsized[0][1][2] : 0.0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    MatrixViews views;
    views.sized = matrices;
    views.unsized = matrices;
    update(views, flags, output);
}

// COMPILE: computeMain
// SPIRV: OpEntryPoint GLCompute
// WGSL: struct _MatrixStorage_float2x3_ColMajorstd430
// WGSL: data{{[_0-9]*}} : array<vec2<f32>, i32(3)>
// WGSL-DAG: var<storage, read> matrices{{[_0-9]*}} : array<_MatrixStorage_float2x3_ColMajorstd430
// WGSL-DAG: var<storage, read_write> flags{{[_0-9]*}} : array<u32>;
// WGSL-DAG: var<storage, read_write> output{{[_0-9]*}} : array<f32>;
// WGSL: flags{{[_0-9]*}}[i32(0)] =
// WGSL: output{{[_0-9]*}}[i32(0)] =
// METAL: struct _MatrixStorage_float2x3_ColMajornatural
// METAL: array<packed_float2, int(3)> data
// METAL: void update{{[_0-9]*}}(const _MatrixStorage_float2x3_ColMajornatural{{[_0-9]*}} device* views_sized{{[_0-9]*}}, const _MatrixStorage_float2x3_ColMajornatural{{[_0-9]*}} device* views_unsized{{[_0-9]*}}, bool device* writableFlags{{[_0-9]*}}, float device* writableOutput{{[_0-9]*}})
// METALLIB: define void @computeMain
// RESULT: 6.0
