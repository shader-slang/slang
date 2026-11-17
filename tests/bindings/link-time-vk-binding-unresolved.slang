// `extern` link-time constants with no default initializer and no linked module that
// defines them cannot resolve to concrete descriptor indices at layout time. When both
// arguments of one attribute are unresolvable, the resolver diagnoses the attribute once.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -entry main -stage compute

extern static const int Slot;
extern static const int SetSlot;

[[vk::binding(Slot, SetSlot)]]
/*CHECK:
      ^^^^^^^ binding attribute requires a concrete value
      ^^^^^^^ an argument of '[[vk::binding(...)]]' or the GLSL 'binding'/'set' layout qualifier is a link-time constant that is not a concrete value here; provide a default or link a module that defines it
*/
RWStructuredBuffer<uint> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    result[threadID.x] = threadID.x;
}
