// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime;
import slang.compute.buffer_utils;
import slang.compute.texture_utils;

[host]
void runBufferTextureExample()
{
    let values = RWStructuredBuffer<float>::alloc(73);
    values.fill(2.0f);
    values.mapInPlace((float value) => value * 3.0f);
    let total = RWStructuredBuffer<float>::alloc(1);
    values.reduceTo(total, (float a, float b) => a + b);
    print(total.readBack(0)); // 438.000000

    let image = RWTexture2D<float>::alloc(19, 9);
    image.fill(1.0f);
    image.mapInPlace((float value) => value + 1.0f);
    image.reduceTo(total, (float a, float b) => a + b);
    print(total.readBack(0)); // 342.000000
}

[host]
void main()
{
    runBufferTextureExample();
}
