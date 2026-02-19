//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv

struct T2 {
    float4 _m0;
};

[[vk::binding(0)]]
cbuffer ubo : register(b0)
//diag: ^^^ see constant buffer definition.
{
    int in_val;
    T2 _z0[]; // error 31215: cannot use unsized type 'T2[]' in a constant buffer.
//diag:^^^ unsized type in constant buffer
//diag:^^^ cannot use unsized type 'T2[]' in a constant buffer.
}

[shader("fragment")]
float4 main() : SV_TARGET
{
    uint x = in_val;
    float4 data = _z0[x]._m0;
    return data;
}
