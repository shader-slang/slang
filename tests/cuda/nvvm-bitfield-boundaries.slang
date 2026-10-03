// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Runtime inputs keep empty/full-width shifts from disappearing during constant folding.
// -91 has low byte0xa5; extracting its high nibble gives -6 when signed and10 otherwise.
// Replacing that nibble with the low nibble of60 gives low byte0xc5, preserving all higher bits.
//TEST_INPUT:ubuffer(data=[0 8 91 60 4 1], stride=4):name=parameters
StructuredBuffer<uint> parameters;
//TEST_INPUT:ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559], stride=4):out,name=outputs
RWStructuredBuffer<uint> outputs;

[noinline]
uint checkScalar<T : __BuiltinIntegerType, let Signed : bool>()
{
    uint zero = parameters[0];
    uint width = parameters[1] * sizeof(T);
    uint nibble = parameters[4];
    uint one = parameters[5];
    T base = T(T(0) - T(parameters[2]));
    T inserted = T(T(parameters[3]));
    T topBit = T(Signed ? T(-1) : T(1));
    T highNibble = T(Signed ? T(-6) : T(10));
    T patched = T(T(-59));
    uint errors = 0;
    errors |= !all(bitfieldExtract(base, zero, zero) == T(0)) ? 1u : 0u;
    errors |= !all(bitfieldExtract(base, width, zero) == T(0)) ? 2u : 0u;
    errors |= !all(bitfieldExtract(base, zero, width) == base) ? 4u : 0u;
    errors |= !all(bitfieldExtract(base, width - one, one) == topBit) ? 8u : 0u;
    errors |= !all(bitfieldExtract(base, nibble, nibble) == highNibble) ? 16u : 0u;
    errors |= !all(bitfieldInsert(base, inserted, zero, zero) == base) ? 32u : 0u;
    errors |= !all(bitfieldInsert(base, inserted, width, zero) == base) ? 64u : 0u;
    errors |= !all(bitfieldInsert(base, inserted, zero, width) == inserted) ? 128u : 0u;
    errors |= !all(bitfieldInsert(base, inserted, nibble, nibble) == patched) ? 256u : 0u;
    return errors;
}

[noinline]
uint checkVector<T : __BuiltinIntegerType, let N : int, let Signed : bool>()
{
    uint zero = parameters[0];
    uint width = parameters[1] * sizeof(T);
    uint nibble = parameters[4];
    uint one = parameters[5];
    vector<T, N> base = vector<T, N>(T(0) - T(parameters[2]));
    vector<T, N> inserted = vector<T, N>(T(parameters[3]));
    vector<T, N> topBit = vector<T, N>(Signed ? T(-1) : T(1));
    vector<T, N> highNibble = vector<T, N>(Signed ? T(-6) : T(10));
    vector<T, N> patched = vector<T, N>(T(-59));
    uint errors = 0;
    errors |= !all(bitfieldExtract(base, zero, zero) == vector<T, N>(0)) ? 1u : 0u;
    errors |= !all(bitfieldExtract(base, width, zero) == vector<T, N>(0)) ? 2u : 0u;
    errors |= !all(bitfieldExtract(base, zero, width) == base) ? 4u : 0u;
    errors |= !all(bitfieldExtract(base, width - one, one) == topBit) ? 8u : 0u;
    errors |= !all(bitfieldExtract(base, nibble, nibble) == highNibble) ? 16u : 0u;
    errors |= !all(bitfieldInsert(base, inserted, zero, zero) == base) ? 32u : 0u;
    errors |= !all(bitfieldInsert(base, inserted, width, zero) == base) ? 64u : 0u;
    errors |= !all(bitfieldInsert(base, inserted, zero, width) == inserted) ? 128u : 0u;
    errors |= !all(bitfieldInsert(base, inserted, nibble, nibble) == patched) ? 256u : 0u;
    vector<T, N> lanes;
    for (int lane = 0; lane < N; ++lane)
        lanes[lane] = T(0) - T(parameters[2]) + T(lane);
    errors |= !all(bitfieldExtract(lanes, zero, width) == lanes) ? 512u : 0u;
    errors |= !all(bitfieldInsert(base, lanes, zero, width) == lanes) ? 1024u : 0u;
    return errors;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputs[0] = checkScalar<int8_t, true>() | checkVector<int8_t, 2, true>() |
        checkVector<int8_t, 3, true>() | checkVector<int8_t, 4, true>();
    outputs[1] = checkScalar<uint8_t, false>() | checkVector<uint8_t, 2, false>() |
        checkVector<uint8_t, 3, false>() | checkVector<uint8_t, 4, false>();
    outputs[2] = checkScalar<int16_t, true>() | checkVector<int16_t, 2, true>() |
        checkVector<int16_t, 3, true>() | checkVector<int16_t, 4, true>();
    outputs[3] = checkScalar<uint16_t, false>() | checkVector<uint16_t, 2, false>() |
        checkVector<uint16_t, 3, false>() | checkVector<uint16_t, 4, false>();
    outputs[4] = checkScalar<int, true>() | checkVector<int, 2, true>() |
        checkVector<int, 3, true>() | checkVector<int, 4, true>();
    outputs[5] = checkScalar<uint, false>() | checkVector<uint, 2, false>() |
        checkVector<uint, 3, false>() | checkVector<uint, 4, false>();
    outputs[6] = checkScalar<int64_t, true>() | checkVector<int64_t, 2, true>() |
        checkVector<int64_t, 3, true>() | checkVector<int64_t, 4, true>();
    outputs[7] = checkScalar<uint64_t, false>() | checkVector<uint64_t, 2, false>() |
        checkVector<uint64_t, 3, false>() | checkVector<uint64_t, 4, false>();
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 3735928559{{$}}
