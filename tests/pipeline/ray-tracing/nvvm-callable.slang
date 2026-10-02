// Callable definitions use a device function ABI and can invoke another dynamic callable.
//TEST:SIMPLE(filecheck=CALLABLE): -target ptx -emit-cuda-via-nvvm -entry callableMain -stage callable -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=CALLABLE): -target ptx -emit-cuda-via-nvvm -entry callableMain -stage callable -capability cuda_sm_8_0 -O3 -o -
// CALLABLE: result code = 0
// CALLABLE: .visible .func __direct_callable__callableMain(
// CALLABLE: _optix_call_direct_callable

struct Payload
{
    float3 value;
    uint index;
};

[shader("callable")]
void callableMain(inout Payload payload)
{
    uint index = payload.index;
    payload.value += float3(1, 2, 3);
    CallShader(index, payload);
    payload.value *= 2;
}

// Empty payload erasure must preserve the shader's effects, even when no value is consumed.
//TEST:SIMPLE(filecheck=EMPTY): -target ptx -emit-cuda-via-nvvm -entry emptyMain -stage raygeneration -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=EMPTY): -target ptx -emit-cuda-via-nvvm -entry emptyMain -stage raygeneration -capability cuda_sm_8_0 -O3 -o -
// EMPTY: result code = 0
// EMPTY: _optix_call_direct_callable

struct Empty {}
struct NestedEmpty { Empty values[2]; }
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void emptyMain()
{
    NestedEmpty payload = {};
    CallShader(DispatchRaysIndex().x, payload);
    output[0] = 41;
}
