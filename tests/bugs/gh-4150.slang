//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1,non-exhaustive):-target spirv -DERROR1
// Using non-exhaustive due to unrelated link-time constant array warning
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2,non-exhaustive):-target spirv -DERROR2
// Using non-exhaustive due to unrelated link-time constant array warning
extern static const int constValue = 1;

struct RWTex<T : __BuiltinRealType, let N : uint>
{
#ifdef ERROR2
    // expect error: cannot define static member with unsized type.
    static [[vk::binding(0, 0)]] vector<T,N> rwtable[];
//CHECK2:                                    ^^^^^^^ cannot instantiate unsized type
//CHECK2:                                    ^^^^^^^ cannot instantiate a variable of unsized type.
#else
    static [[vk::binding(0, 0)]] vector<T,N> rwtable[4];
#endif
    static vector<T, N> get(uint image_index) { return rwtable[image_index]; }
}

struct Push
{
    uint image_id;
};
[[vk::push_constant]] Push p;
RWStructuredBuffer<float3> output;
[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 pixel_i : SV_DispatchThreadID)
{
    output[0] =
#ifdef ERROR1
        // expect error: trying to specialize RWTex, which has two arguments, with only one argument.
        RWTex<float3>::get(p.image_id);
/*CHECK1:
        ^^^^^ cannot specialize generic
        ^^^^^ cannot specialize generic 'RWTex' with the provided arguments.
*/
#else
        RWTex<float, 3>::get(p.image_id);
#endif
    static float sa1[];
//CHECK1:        ^^^ cannot instantiate unsized type
//CHECK1:        ^^^ cannot instantiate a variable of unsized type.

    float sa2[];
//CHECK1: ^^^ cannot instantiate unsized type
//CHECK1: ^^^ cannot instantiate a variable of unsized type.

    static float sa3[] = { 1, 2, 3 }; // should be ok.

    float sa4[] = { 1, 2, 3 }; // should be ok.

    float sa5[constValue]; // should be ok.
}
