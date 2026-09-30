// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Runtime-loaded edges for unsigned abs, integer sign, raw Half abs, and canonical reductions.
//TEST_INPUT: ubuffer(data=[4294967295 2147483648 65025 32768 4290772993 0 4294443008], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;
//TEST_INPUT: ubuffer(data=[0 0], stride=8):name=globalValue
RWStructuredBuffer<Atomic<uint64_t>> globalValue;
//TEST_INPUT: ubuffer(data=[324508639 99 99 99 99 99 99 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;
groupshared uint shared32;
groupshared uint64_t shared64;
groupshared int64_t sharedSigned64;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint allBits = inputWords[0];
    int minimum = int(inputWords[1]);
    uint64_t wide = (uint64_t(allBits) << 32) | uint64_t(allBits);
    int64_t wideMinimum = int64_t(uint64_t(inputWords[1]) << 32);
    uint errors = 0;
    errors |= abs(uint8_t(allBits)) != uint8_t(allBits) ? 1u : 0u;
    errors |= abs(uint16_t(allBits)) != uint16_t(allBits) ? 2u : 0u;
    errors |= abs(allBits) != allBits ? 4u : 0u;
    errors |= abs(wide) != wide ? 8u : 0u;
    errors |= abs(int8_t(minimum >> 24)) != int8_t(-128) ? 16u : 0u;
    errors |= abs(int16_t(minimum >> 16)) != int16_t(-32768) ? 32u : 0u;
    errors |= abs(minimum) != minimum ? 64u : 0u;
    errors |= abs(wideMinimum) != wideMinimum ? 128u : 0u;
    errors |= sign(int8_t(minimum >> 24)) != -1 ? 256u : 0u;
    errors |= sign(int16_t(minimum >> 16)) != -1 ? 512u : 0u;
    errors |= sign(minimum) != -1 || sign(wideMinimum) != -1 ? 1024u : 0u;
    errors |= sign(int(inputWords[5])) != 0 || sign(int64_t(inputWords[5]) + 1) != 1 ? 2048u : 0u;
    errors |= uint(bit_cast<uint16_t>(abs(bit_cast<half>(uint16_t(inputWords[2] - 512))))) != 31745 ? 4096u : 0u;
    outputBuffer[1] = errors;
    half negativeNaN = bit_cast<half>(uint16_t(inputWords[2]));
    outputBuffer[2] = uint(bit_cast<uint16_t>(abs(negativeNaN)));
    half negativeZero = bit_cast<half>(uint16_t(inputWords[3]));
    outputBuffer[3] = uint(bit_cast<uint16_t>(abs(negativeZero)));
    outputBuffer[4] = uint(sign(negativeNaN) == 0 && sign(negativeZero) == 0 &&
        sign(asfloat(inputWords[4])) == 0 && sign(asdouble(inputWords[5], inputWords[6])) == 0);

    // One thread owns initialization and every observation; no cross-thread ordering is implied.
    shared32 = allBits;
    __atomic_reduce_inc(shared32);
    uint atomicErrors = shared32 != 0 ? 1u : 0u;
    __atomic_reduce_dec(shared32);
    atomicErrors |= shared32 != allBits ? 2u : 0u;
    shared64 = wide;
    __atomic_reduce_inc(shared64);
    atomicErrors |= shared64 != 0 ? 4u : 0u;
    __atomic_reduce_dec(shared64);
    atomicErrors |= shared64 != wide ? 8u : 0u;
    sharedSigned64 = wideMinimum;
    __atomic_reduce_dec(sharedSigned64);
    atomicErrors |= sharedSigned64 != int64_t(wide >> 1) ? 16u : 0u;
    __atomic_reduce_inc(sharedSigned64);
    atomicErrors |= sharedSigned64 != wideMinimum ? 32u : 0u;
    __atomic_reduce_xor(shared64, wide);
    __atomic_reduce_add(shared64, uint64_t(5));
    __atomic_reduce_sub(shared64, uint64_t(2));
    __atomic_reduce_max(shared64, uint64_t(7));
    __atomic_reduce_min(shared64, uint64_t(6));
    __atomic_reduce_and(shared64, uint64_t(3));
    __atomic_reduce_or(shared64, uint64_t(8));
    atomicErrors |= shared64 != 10 ? 64u : 0u;
    outputBuffer[5] = atomicErrors;
    globalValue[0].store(wide);
    globalValue[0].reduceInc();
    uint globalErrors = globalValue[0].load() != 0 ? 1u : 0u;
    globalValue[0].reduceDec();
    globalErrors |= globalValue[0].load() != wide ? 2u : 0u;
    outputBuffer[6] = globalErrors;
}

// CHECK: type: uint
// CHECK-NEXT: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 32257{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 3735928559{{$}}
