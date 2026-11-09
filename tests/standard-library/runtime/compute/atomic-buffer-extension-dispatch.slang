// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;

extension RWStructuredBuffer<Atomic<uint>>
{
    // The dispatched lambda's reflection name includes its extension target, including Atomic.
    [host]
    void storeValue(uint value)
    {
        Grid(1u).dispatch((uint index) => this[index].store(value));
    }
}

[host]
void main()
{
    let counters = RWStructuredBuffer<Atomic<uint>>::alloc(1);
    counters.storeValue(42u);
    let result = RWStructuredBuffer<uint>::alloc(1);
    Grid(1u).dispatch(
        (uint index) =>
        { result[index] = counters[index].load(); });
    assert(readBack(result, 0) == 42u);
}
