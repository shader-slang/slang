// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm): -api cuda

import slang.tensor;
import slang.numerics;
import slang.runtime.core.simple_assert;

[host]
void main()
{
    // Leave nonzero padding in a larger allocation; operations must honor the logical shape.
    var values = Tensor<float, 2>(uint2(32u, 32u));
    values.fill(1000.0f);
    values.resize(uint2(17u, 9u));
    values.fill(1.0f);
    let vectors =
        values.map((uint2 pixel, float value) => float2(value + float(pixel.x), float(pixel.y)));
    float2 last = vectors.readBack(152u);
    assert(last.x == 17.0f && last.y == 8.0f);
    assert(values.reduce(0.0f, (float a, float b) => a + b).readBack(0) == 153.0f);
    // sum_(x=1..17,y=0..8) (x^2 + y^2) = 9*1785 + 17*204.
    assert(innerProduct(vectors, vectors).readBack(0) == 19533.0f);
    let copied = values.copy();
    values.fill(7.0f);
    assert(copied.readBack(152u) == 1.0f);
    let empty = Tensor<float, 2>(uint2(0u, 9u));
    assert(empty.reduce(0.0f, (float a, float b) => a + b).readBack(0) == 0.0f);
    assert(innerProduct(empty, empty).readBack(0) == 0.0f);
}
