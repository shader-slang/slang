// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api cuda
// TEST:INTERPRET(hgvm): -api d3d12

import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;

struct BufferGradient : IDifferentiablePtrType
{
    typealias Differential = BufferGradient;
    no_diff RWStructuredBuffer<float> buffer;
}

struct BufferRef : IDifferentiablePtrType
{
    typealias Differential = BufferGradient;
    no_diff RWStructuredBuffer<float> buffer;
    no_diff uint index;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void copyPairs(
    uniform DifferentialPair<float> values,
    uniform DifferentialPtrPair<BufferRef> pointers,
    RWStructuredBuffer<float> result)
{
    result[0] = values.p;
    result[1] = values.d;
    result[2] = pointers.p.buffer[pointers.p.index];
    result[3] = pointers.d.buffer[0];
}

[host]
void main()
{
    let primal = RWStructuredBuffer<float>::alloc(2);
    let differential = RWStructuredBuffer<float>::alloc(1);
    let result = RWStructuredBuffer<float>::alloc(4);
    Grid(1u).dispatch(
        (uint i) =>
        {
            primal[0] = -1.0f;
            primal[1] = 3.0f;
            differential[0] = 11.0f;
        });

    // Read back the inputs so constant folding cannot eliminate the captured pair.
    let values = diffPair(readBack(primal, 1), readBack(differential, 0));
    let pointers =
        DifferentialPtrPair<BufferRef>(BufferRef(primal, 1u), BufferGradient(differential));
    assert(values.p == 3.0f && values.d == 11.0f);

    Grid(1u).dispatch(
        (uint i) =>
        {
            result[0] = values.p;
            result[1] = values.d;
            result[2] = pointers.p.buffer[pointers.p.index];
            result[3] = pointers.d.buffer[0];
        });
    assert(readBack(result, 0) == 3.0f);
    assert(readBack(result, 1) == 11.0f);
    assert(readBack(result, 2) == 3.0f);
    assert(readBack(result, 3) == 11.0f);

    // Direct uniform arguments must use the same field keys as closure captures.
    dispatch(copyPairs, uint3(1u))(values, pointers, result);
    assert(readBack(result, 0) == 3.0f);
    assert(readBack(result, 1) == 11.0f);
    assert(readBack(result, 2) == 3.0f);
    assert(readBack(result, 3) == 11.0f);
}
