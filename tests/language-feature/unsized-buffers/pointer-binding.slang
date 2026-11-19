//TEST(compute,64-bit):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -output-using-type
//TEST(compute,64-bit):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -output-using-type
//TEST(compute,64-bit):COMPARE_COMPUTE(filecheck-buffer=CHECK): -llvm -compute -output-using-type

// This test exercises the CPU/CUDA/LLVM prelude ABI with buffers embedded as raw addresses.
// Metal/WGSL use native resource bindings instead, covered by operations.slang.
// Supply a parameter payload containing buffer addresses directly. The pointer uniforms
// use the test runner's ordinary-data binding path; the RHI resource-binding path currently
// assumes that every raw buffer also has a count. The payload has no room for such counts:
// each buffer pointer is followed by a scalar and four bytes of alignment padding.
//TEST_INPUT:ubuffer(data=[11 22], stride=4):name=sourceAddress
uint64_t sourceAddress;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=workingAddress
uint64_t workingAddress;
//TEST_INPUT:ubuffer(data=[100 200], stride=4):name=bytesAddress
uint64_t bytesAddress;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=destinationAddress
uint64_t destinationAddress;

struct Params
{
    __UnsizedStructuredBuffer<uint> source;
    uint sourceMarker;
    __UnsizedRWStructuredBuffer<uint> working;
    uint workingMarker;
    __UnsizedByteAddressBuffer bytes;
    uint bytesMarker;
    __UnsizedRWByteAddressBuffer destination;
    uint destinationMarker;
}

//TEST_INPUT:ubuffer(data=[sourceAddress 101 0 workingAddress 202 0 bytesAddress 303 0 destinationAddress 404 0], stride=64):name=params
Params* params;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    params->working[0] = params->source[0] + params->bytes.Load(0);
    params->destination.Store(4, params->working[0] + params->source[1]);
    output[0] = params->working[0];
    output[1] = params->destination.Load(4);
    output[2] = params->sourceMarker;
    output[3] = params->workingMarker;
    output[4] = params->bytesMarker;
    output[5] = params->destinationMarker;
    output[6] = sourceAddress != 0 && workingAddress != 0 &&
        bytesAddress != 0 && destinationAddress != 0;
}

// CHECK: 111
// CHECK: 133
// CHECK: 101
// CHECK: 202
// CHECK: 303
// CHECK: 404
// CHECK: 1
