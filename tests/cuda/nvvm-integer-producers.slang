// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=IR): -target ptx -stage compute -entry computeMain -O0 -capability cuda_sm_8_0 -emit-cuda-via-nvvm -dump-ir-after checkUnsupportedInst -validate-ir -o -

// Public bit APIs compose explicitly named same-width LLVM primitives with ordinary Slang
// casts and signed-high arithmetic. The runtime fixtures independently check their values.
// IR: ### AFTER checkUnsupportedInst:
// IR-DAG: GenericAsm("llvm.ctpop", {{%[a-zA-Z0-9_]+}})
// IR-DAG: GenericAsm("llvm.bitreverse", {{%[a-zA-Z0-9_]+}})
// IR-DAG: GenericAsm("llvm.ctlz", {{%[a-zA-Z0-9_]+}}, false)
// IR-DAG: GenericAsm("llvm.cttz", {{%[a-zA-Z0-9_]+}}, false)
// IR-DAG: bitnot(
// IR-DAG: sub(
// IR-DAG: intCast(

[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int signedValue,
    uniform uint64_t wideValue)
{
    destination[0] = countbits(wideValue);
    destination[1] = uint(reversebits(wideValue));
    destination[2] = firstbithigh(signedValue);
    destination[3] = firstbitlow(wideValue);
}
