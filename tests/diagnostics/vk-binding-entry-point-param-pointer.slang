//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

// Regression test for https://github.com/shader-slang/slang/issues/11857.
//
// A `[[vk::binding(...)]]` on a raw pointer entry-point parameter cannot be
// honored: a raw pointer is a 64-bit buffer-device-address value laid out in
// the push-constant block, not a descriptor resource, so it never consumes a
// `DescriptorTableSlot`/`SubElementRegisterSpace` that the annotation could
// position. The compiler must therefore warn (E38010) that the attribute is
// ignored rather than silently accepting it.

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(1, 2)]]
    uint* p)
//CHECK:  ^ modifier on entry point parameter is unsupported
//CHECK:  ^ attribute '[[vk::binding(...)]]' on parameter 'p' is unsupported on entry point parameters and will be ignored
{
    output[0] = 0;
}
