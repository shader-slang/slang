// An `extern` link-time constant with no default initializer and no linked module that
// defines it cannot resolve to a concrete descriptor index at layout time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -entry main -stage compute

extern static const int Slot;

[[vk::binding(Slot, 1)]]
/*CHECK:
      ^^^^^^^ binding attribute requires a concrete value
      ^^^^^^^ an argument of '[[vk::binding(...)]]' is a link-time constant that is not a concrete value here; provide a default or link a module that defines it
*/
RWStructuredBuffer<uint> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    result[threadID.x] = threadID.x;
}
