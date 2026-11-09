// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm): -api cuda

import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;

// The dispatched lambda's reflection path must spell operator() as source and keep the
// synthesized lambda declaration name a valid identifier, even inside a generic type.
struct DispatchContext<let Count : int>
{
    RWStructuredBuffer<float> values;

    [host]
    void operator()()
    {
        let destination = values;
        Grid(uint(Count))
            .dispatch(
                (uint index) =>
                { destination[index] = float(index * 3 + 1); });
    }
}

[host]
void main()
{
    let values = RWStructuredBuffer<float>::alloc(65);
    let context = DispatchContext<65>(values);
    context();
    assert(readBack(values, 0) == 1.0f);
    assert(readBack(values, 64) == 193.0f);
}
