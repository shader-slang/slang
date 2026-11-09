// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api d3d12 -entry main
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api vulkan -entry main

// TEST:INTERPRET(hgvm,filecheck=GENERIC): -api d3d12 -entry genericMain
// TEST:INTERPRET(hgvm,filecheck=GENERIC): -api vulkan -entry genericMain

import slang.runtime.compute.grid;

[host]
uint hostHelper(uint value)
{
    return value + 1;
}

[host]
uint genericHostHelper<T>(uint value)
{
    return hostHelper(value);
}

[host]
void main()
{
    let output = RWStructuredBuffer<uint>::alloc(1);
    // The generic IFunc call defers the callback's implementation until device linking.
    Grid(1u).dispatch(
        (uint index) =>
        { output[index] = hostHelper(index); });
    print(output.readBack(0));
}

[host]
void genericMain()
{
    let output = RWStructuredBuffer<uint>::alloc(1);
    Grid(1u).dispatch(
        (uint index) =>
        { output[index] = genericHostHelper<uint>(index); });
    print(output.readBack(0));
}

// CHECK: result code = 1
// CHECK: host function 'hostHelper' is referenced by a device program
// GENERIC: result code = 1
// GENERIC: host function 'genericHostHelper' is referenced by a device program
