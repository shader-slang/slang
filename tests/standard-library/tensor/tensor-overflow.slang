// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm,filecheck=CHECK): -api vulkan -entry constructorOverflow
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api vulkan -entry resizeOverflow
// CHECK: result code = 1
// CHECK: assertion failed

import slang.tensor;

[host]
void constructorOverflow()
{
    // Without validation, this product wraps to zero and incorrectly creates an empty tensor.
    let tensor = Tensor<uint, 2>(uint2(65536, 65536));
}

[host]
void resizeOverflow()
{
    var tensor = Tensor<uint, 2>(uint2(1, 1));
    tensor.resize(uint2(65536, 65536));
}
