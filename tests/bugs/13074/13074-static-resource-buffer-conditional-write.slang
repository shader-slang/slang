// DXC cannot represent a structured or byte-address buffer in mutable local storage. Selecting
// between two such buffers leaves that storage after specialization. Metal represents the same
// buffer values as device pointers, so those locals remain legal there.

//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=DXIL): -O0 -target dxil -entry main -stage compute -validate-ir
//TEST:SIMPLE(filecheck=METAL): -O0 -target metal -entry main -stage compute -validate-ir
//TEST:SIMPLE(filecheck=METAL): -O0 -minimum-slang-optimization -target metal -entry main -stage compute -validate-ir

StructuredBuffer<float4> structuredA;
StructuredBuffer<float4> structuredB;
ByteAddressBuffer bytesA;
ByteAddressBuffer bytesB;
RWStructuredBuffer<uint> output;
uniform bool selectSecondBuffer;

static StructuredBuffer<float4> selectedStructured = structuredA;
//DXIL:                         ^^^^^^^^^^^^^^^^^^ target cannot represent an opaque value in mutable local storage
//DXIL:                         ^^^^^^^^^^^^^^^^^^ mutable local storage contains an opaque value of type 'StructuredBuffer<vector<float,4>>', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable
static ByteAddressBuffer selectedBytes = bytesA;
//DXIL:                  ^^^^^^^^^^^^^ target cannot represent an opaque value in mutable local storage
//DXIL:                  ^^^^^^^^^^^^^ mutable local storage contains an opaque value of type 'ByteAddressBuffer', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    if (selectSecondBuffer)
    {
        selectedStructured = structuredB;
        selectedBytes = bytesB;
    }

    uint index = threadID.x;
    output[index] = asuint(selectedStructured[index].x) + selectedBytes.Load(index * 4);
}

//METAL: packed_float4 device* selectedStructured_
//METAL: uint32_t device* selectedBytes_
//METAL-NOT: error
