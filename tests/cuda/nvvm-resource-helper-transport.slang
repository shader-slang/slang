//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST_INPUT:ubuffer(data=[11 23 37 53], stride=4):name=first
StructuredBuffer<uint> first;
//TEST_INPUT:ubuffer(data=[71 83 97 109], stride=4):name=second
StructuredBuffer<uint> second;
//TEST_INPUT:ubuffer(data=[127 139 151 163], stride=4):name=raw
ByteAddressBuffer raw;
//TEST_INPUT:ubuffer(data=[193 0 0 0 0 0 0 0 0 0 197], stride=4):out,name=output
RWStructuredBuffer<uint> output;
//TEST_INPUT:ubuffer(data=[211 0 223], stride=4):out,name=rawOutput
RWByteAddressBuffer rawOutput;

[noinline] T copyValue<T>(T value) { return value; }
[noinline] void initialize<T>(out T value, T source) { value = copyValue(source); }
[noinline] void replace<T>(inout T value, T source) { initialize(value, source); }
[noinline] T readValue<T>(__constref T value) { return value; }
[noinline] T readForward<T>(__constref T value) { return readValue(value); }

struct Leaf { StructuredBuffer<uint> buffer; ByteAddressBuffer bytes; }
struct Nested { Leaf leaves[2]; RWStructuredBuffer<uint> destination; }
typedef StructuredBuffer<uint> BufferPair[2];
[noinline] BufferPair copyPair(BufferPair pair) { return pair; }
[noinline] uint observe(__constref Nested value, uint index)
{
    return readForward(value.leaves[index].buffer)[index] +
        readForward(value.leaves[index].bytes).Load(index * 4);
}
[noinline] void mutate(inout Nested value, uint index)
{
    replace(value.leaves[index].buffer, second);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    StructuredBuffer<uint> a;
    initialize(a, first);
    output[1] = readForward(a)[1];
    replace(a, second);
    output[2] = readForward(a)[2];
    BufferPair pair = { first, second };
    pair = copyPair(pair);
    output[3] = pair[0][0] + pair[1][3];
    replace(pair[1], first);
    output[4] = pair[1][2];
    Nested value;
    initialize(value.leaves[0].buffer, first);
    initialize(value.leaves[1].buffer, first);
    initialize(value.leaves[0].bytes, raw);
    initialize(value.leaves[1].bytes, raw);
    initialize(value.destination, output);
    mutate(value, 1);
    value.destination[5] = observe(value, 1);
    RWByteAddressBuffer bytes;
    initialize(bytes, rawOutput);
    replace(bytes, rawOutput);
    readForward(bytes).Store(4, 181);
    value.destination[6] = readForward(bytes).Load(4);
    value.destination[7] = observe(value, 0);
    value.destination[8] = bytes.Load(0);
    value.destination[9] = bytes.Load(8);
}
// CHECK: 193
// CHECK: 23
// CHECK: 97
// CHECK: 120
// CHECK: 37
// CHECK: 222
// CHECK: 181
// CHECK: 138
// CHECK: 211
// CHECK: 223
// CHECK: 197
