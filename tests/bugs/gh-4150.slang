//TEST:SIMPLE(filecheck=CHECK1):-target spirv -DERROR1
//TEST:SIMPLE(filecheck=CHECK2):-target spirv -DERROR2

// CHECK1: error 30075
// CHECK2: error 30071

struct RWTex<T : __BuiltinRealType, let N : uint>
{
#ifdef ERROR2
    // expect error: cannot define static member with unsized type.
    static [[vk::binding(0, 0)]] vector<T,N> rwtable[];
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
#else
    RWTex<float,3>::get(p.image_id);
#endif
}
