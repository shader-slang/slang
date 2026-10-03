//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3

struct Inner { uint8_t tag; uint value; }
struct Outer { uint8_t tag; Inner inner; }

struct BooleanRecord
{
    bool2 pair;
    bool3 triple;
    bool4 quad;
}

[noinline]
BooleanRecord readBooleanRecord(BooleanRecord values[3], uint index)
{
    return values[index];
}

struct BooleanEnvelope
{
    uint8_t tag;
    BooleanRecord records[2];
    uint tail;
}

[noinline]
BooleanEnvelope readBooleanEnvelope(BooleanEnvelope values[2], uint index)
{
    return values[index];
}

uint observeBooleanRecord(BooleanRecord value)
{
    return uint(value.pair.x) | (uint(value.pair.y) << 1) |
        (uint(value.triple.x) << 2) | (uint(value.triple.y) << 3) |
        (uint(value.triple.z) << 4) | (uint(value.quad.x) << 5) |
        (uint(value.quad.y) << 6) | (uint(value.quad.z) << 7) |
        (uint(value.quad.w) << 8);
}

[noinline]
float readLane(float values[256], uint8_t index)
{
    return values[index];
}

[noinline]
Outer readRecord(Outer values[3], uint index)
{
    return values[index];
}

//TEST_INPUT:ubuffer(data=[200 2 32], stride=4):name=parameters
StructuredBuffer<uint> parameters;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputs
RWStructuredBuffer<uint> outputs;
// CHECK: 200
// CHECK-NEXT: 6896
// CHECK-NEXT: 37
// CHECK-NEXT: 79
// CHECK-NEXT: 210
// CHECK-NEXT: 45
// CHECK-NEXT: 0
// CHECK-NEXT: 288
[numthreads(1, 1, 1)]
void computeMain()
{
    float values[256];
    for (uint i = 0; i < 256; ++i)
        values[i] = float(i);
    uint8_t index = uint8_t(parameters[0]);
    outputs[0] = uint(readLane(values, index));
    float total = 0;
    for (uint i = 0; i < parameters[2]; ++i)
    {
        total += readLane(values, index);
        values[uint(index)] += 1;
    }
    outputs[1] = uint(total); // 32 * 200 + 32 * 31 / 2

    Outer records[3] = {
        {uint8_t(1), {uint8_t(2), 3}},
        {uint8_t(4), {uint8_t(5), 6}},
        {uint8_t(7), {uint8_t(11), 19}}
    };
    Outer before = readRecord(records, parameters[1]);
    records[parameters[1]].inner.value = 61;
    Outer after = readRecord(records, parameters[1]);
    outputs[2] = uint(before.tag) + uint(before.inner.tag) + before.inner.value;
    outputs[3] = uint(after.tag) + uint(after.inner.tag) + after.inner.value;

    // A dynamic array read snapshots the complete record, including Boolean vector lanes.
    // Preserve that snapshot when the source record is subsequently replaced.
    BooleanRecord booleanRecords[3] = {
        {bool2(false), bool3(false), bool4(false)},
        {bool2(true), bool3(true), bool4(true)},
        {bool2(false, true), bool3(false, false, true), bool4(false, true, true, false)}
    };
    uint errors = 0;
    for (uint i = 0; i < 3; ++i)
    {
        uint selected = (parameters[1] + i) % 3;
        uint expected = selected == 0 ? 0 : (selected == 1 ? 511 : 210);
        errors |= uint(observeBooleanRecord(readBooleanRecord(booleanRecords, selected)) != expected);
    }
    outputs[6] = errors;
    BooleanEnvelope envelopes[2] = {
        {uint8_t(3), {booleanRecords[0], booleanRecords[1]}, 31},
        {uint8_t(7), {booleanRecords[2], booleanRecords[0]}, 71}
    };
    BooleanEnvelope envelope = readBooleanEnvelope(envelopes, parameters[1] - 1);
    outputs[7] = uint(envelope.tag) + envelope.tail +
        observeBooleanRecord(envelope.records[parameters[1] - 2]);

    BooleanRecord booleanBefore = readBooleanRecord(booleanRecords, parameters[1]);
    booleanRecords[parameters[1]] = {
        bool2(true, false), bool3(true, true, false), bool4(true, false, false, false)};
    BooleanRecord booleanAfter = readBooleanRecord(booleanRecords, parameters[1]);
    outputs[4] = observeBooleanRecord(booleanBefore);
    outputs[5] = observeBooleanRecord(booleanAfter);
}
