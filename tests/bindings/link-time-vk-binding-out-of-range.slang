// A binding/set value that RESOLVES must land in the descriptor-index domain a Vulkan
// literal can carry. A negative constant (whether a default or a linked override) is
// diagnosed as out-of-range and reads as index 0; a link-time expression whose wrapping
// fold passes INT64_MAX (Big * Big) resolves to a deterministic wrapped negative and is
// diagnosed the same way — which also pins the two's-complement wrap semantics of the
// polynomial fold.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -entry main -stage compute

export extern static const int NegSlot = -1;
export extern static const int64_t Big = 3037000500;

[[vk::binding(NegSlot, 1)]]
/*CHECK:
      ^^^^^^^ binding attribute resolved to an out-of-range descriptor index
      ^^^^^^^ an argument of '[[vk::binding(...)]]' or the GLSL 'binding'/'set' layout qualifier resolved to '-1', outside the valid descriptor index range [0, 4294967295]
*/
RWStructuredBuffer<uint> negResult;

[[vk::binding(Big * Big, 2)]]
/*CHECK:
      ^^^^^^^ binding attribute resolved to an out-of-range descriptor index
      ^^^^^^^ an argument of '[[vk::binding(...)]]' or the GLSL 'binding'/'set' layout qualifier resolved to '-9223372036709301616', outside the valid descriptor index range [0, 4294967295]
*/
RWStructuredBuffer<uint> wrapResult;

// Both arguments out of range: one diagnostic per attribute, reporting the binding
// argument's value.
[[vk::binding(NegSlot, NegSlot)]]
/*CHECK:
      ^^^^^^^ binding attribute resolved to an out-of-range descriptor index
      ^^^^^^^ an argument of '[[vk::binding(...)]]' or the GLSL 'binding'/'set' layout qualifier resolved to '-1', outside the valid descriptor index range [0, 4294967295]
*/
RWStructuredBuffer<uint> bothResult;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    negResult[threadID.x] = threadID.x;
    wrapResult[threadID.x] = threadID.x;
    bothResult[threadID.x] = threadID.x;
}
