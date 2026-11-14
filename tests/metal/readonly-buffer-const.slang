// Read-only buffers are emitted as `const device` pointers on Metal, so the Metal compiler knows
// the shader never writes them (#14154). This holds wherever the buffer type is spelled: global
// parameters, the kernel context, helper parameters, argument-buffer fields and descriptor handles.
// `uniform` entry-point parameters are checked in readonly-buffer-const-entry-point.slang.
// A temporary that holds the address of an element of a read-only buffer points to `const` too.
// Writable buffers stay pointers to mutable memory.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -mtl -shaderobj -output-using-type

// METALLIB: @computeMain

struct Pair
{
    float x;
    float y;
};

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=8):name=pairs
StructuredBuffer<Pair> pairs;

//TEST_INPUT:ubuffer(data=[5 6 7 8]):name=words
ByteAddressBuffer words;

struct Params
{
    StructuredBuffer<float> scales;
    ByteAddressBuffer raw;
    RWStructuredBuffer<float> scratch;
};

//TEST_INPUT: set params = new Params{ ubuffer(data=[10.0 20.0], stride=4), ubuffer(data=[100 200]), ubuffer(data=[0 0], stride=4) }
ParameterBlock<Params> params;

//TEST_INPUT: set handle = ubuffer(data=[1000.0 2000.0], stride=4)
uniform StructuredBuffer<float>.Handle handle;

// A matrix element is loaded through a temporary that holds its address.
//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0], stride=36):name=mats
StructuredBuffer<float3x3> mats;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=rawOutput
RWByteAddressBuffer rawOutput;

// CHECK-DAG: struct Params{{.*}}
// CHECK-DAG: const float device* scales_{{[0-9]+}};
// CHECK-DAG: const uint32_t device* raw_{{[0-9]+}};
// CHECK-DAG: {{^ *}}float device* scratch_{{[0-9]+}};

// CHECK-DAG: float sumPair{{.*}}(const Pair{{.*}} device* {{.*}}, uint
[noinline]
float sumPair(StructuredBuffer<Pair> buffer, uint index)
{
    return buffer[index].x + buffer[index].y;
}

// CHECK-DAG: uint readWord{{.*}}(const uint32_t device* {{.*}}, uint
[noinline]
uint readWord(ByteAddressBuffer buffer, uint index)
{
    return buffer.Load(index * 4);
}

// CHECK-DAG: {{\[\[}}kernel{{\]\]}} void computeMain
// CHECK-DAG: const Pair{{.*}} device* pairs_{{[0-9]+}} {{\[\[}}buffer(
// CHECK-DAG: const uint32_t device* words_{{[0-9]+}} {{\[\[}}buffer(
// CHECK-DAG: const float device* handle_{{[0-9]+}} {{\[\[}}buffer(
// CHECK-DAG: const _MatrixStorage_float3x3{{[A-Za-z0-9_]*}} device* mats_{{[0-9]+}} {{\[\[}}buffer(
// CHECK-DAG: {{^ *}}const _MatrixStorage_float3x3{{[A-Za-z0-9_]*}} device* {{[A-Za-z0-9_]+}} = {{.*}}mats_{{[0-9]+}}+
// CHECK-DAG: {{[(,] }}float device* outputBuffer_{{[0-9]+}} {{\[\[}}buffer(
// CHECK-DAG: {{[(,] }}uint32_t device* rawOutput_{{[0-9]+}} {{\[\[}}buffer(

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint tid: SV_GroupIndex)
{
    params.scratch[0] = params.scales[1];
    rawOutput.Store(0, readWord(params.raw, 1));

    // BUFFER: 3.0
    outputBuffer[0] = sumPair(pairs, 0);
    // BUFFER: 6.0
    outputBuffer[1] = float(readWord(words, 1));
    // BUFFER: 20.0
    outputBuffer[2] = params.scratch[0];
    // BUFFER: 200.0
    outputBuffer[3] = float(params.raw.Load(4));
    // BUFFER: 2000.0
    outputBuffer[4] = handle[1];
    // BUFFER: 10.25
    outputBuffer[5] = pairs[1].y * 2.5 + 0.25;
    // BUFFER: 45.0
    float3x3 m = mats[0];
    outputBuffer[6] = dot(m[0] + m[1] + m[2], float3(1.0, 1.0, 1.0));
}
