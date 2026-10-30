//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// `[vk::offset]` is a byte offset stored as a 32-bit signed integer, so a
// negative value or a value above 2147483647 is an error on a struct field
// and on an entry-point parameter.

struct S
{
    [vk::offset(-4)] int a;
//CHECK: ^^^^^^ 'vk::offset' out of range
//CHECK: ^^^^^^ '[vk::offset]' requires a byte offset from 0 to 2147483647, but the offset is -4
}

[vk::push_constant]
S pc;

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain([vk::offset(-8)] uniform int b, [vk::offset(4294967295)] uniform int c)
//CHECK:              ^^^^^^ 'vk::offset' out of range
//CHECK:              ^^^^^^ '[vk::offset]' requires a byte offset from 0 to 2147483647, but the offset is -8
//CHECK:                                              ^^^^^^ 'vk::offset' out of range
//CHECK:                                              ^^^^^^ '[vk::offset]' requires a byte offset from 0 to 2147483647, but the offset is 4294967295
{
    outputBuffer[0] = pc.a + b + c;
}
