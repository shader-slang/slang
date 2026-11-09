// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
uint main(uint count)
{
    // A runtime ID must never be serialized as an integer immediate.
    return hgbc_asm { result : $$uint = OpIntLit $count; };
}

// CHECK: non-constant or out-of-range hgbc_asm immediate operands
