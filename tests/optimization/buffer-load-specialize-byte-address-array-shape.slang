//TEST:SIMPLE(filecheck=SHAPE23): -target spirv-asm -O0 -dump-ir-before legalizeByteAddressBufferOps
//TEST:SIMPLE(filecheck=SHAPE32): -target spirv-asm -O0 -dump-ir-before legalizeByteAddressBufferOps

// Both calls pass a `Frame` to `project`. The specialized callees load differently shaped arrays,
// so they need different strides to fetch the `Frame` at `[i][j]`.

// The two matrices make `Frame` large enough for buffer-load argument specialization.
struct Frame
{
    float4x4 view;
    float4x4 projection;
    float4 position;
};

ByteAddressBuffer frames;
RWStructuredBuffer<float4> output;

[noinline]
float4 project(Frame frame)
{
    return mul(frame.projection, frame.position);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint offset = id.x * 1024;
    uint i = id.y;
    uint j = id.z;
    output[2 * id.x] = project(frames.Load<Frame[2][3]>(offset)[i][j]);
    output[2 * id.x + 1] = project(frames.Load<Frame[3][2]>(offset)[i][j]);
}

// Follow each call into its callee and check the reconstructed load there. Separate prefixes
// allow the callees to be defined in either order.
// SHAPE23: ### BEFORE legalizeByteAddressBufferOps:
// SHAPE23: func %main :
// SHAPE23-NOT: {{^}}}
// SHAPE23: call [[FIRST:%project[^ (]*]](%offset, %i, %j)
// SHAPE23: func [[FIRST]] : Func(Vec(Float, 4 : Int), UInt, UInt, UInt)
// SHAPE23-NEXT: {
// SHAPE23-NEXT: block
// SHAPE23-NEXT: param [[OFFSET:%[0-9]+]] : UInt,
// SHAPE23-NEXT: param %{{[0-9]+}} : UInt,
// SHAPE23-NEXT: param %{{[0-9]+}} : UInt):
// SHAPE23-NEXT: : Array(Array(%Frame, 2 : Int), 3 : Int) = byteAddressBufferLoad(%frames, [[OFFSET]], 0 : UInt)

// SHAPE32: ### BEFORE legalizeByteAddressBufferOps:
// SHAPE32: func %main :
// SHAPE32-NOT: {{^}}}
// SHAPE32: call %project{{[^ (]*}}(%offset, %i, %j)
// SHAPE32-NOT: {{^}}}
// SHAPE32: call [[SECOND:%project[^ (]*]](%offset, %i, %j)
// SHAPE32: func [[SECOND]] : Func(Vec(Float, 4 : Int), UInt, UInt, UInt)
// SHAPE32-NEXT: {
// SHAPE32-NEXT: block
// SHAPE32-NEXT: param [[OFFSET:%[0-9]+]] : UInt,
// SHAPE32-NEXT: param %{{[0-9]+}} : UInt,
// SHAPE32-NEXT: param %{{[0-9]+}} : UInt):
// SHAPE32-NEXT: : Array(Array(%Frame, 3 : Int), 2 : Int) = byteAddressBufferLoad(%frames, [[OFFSET]], 0 : UInt)
