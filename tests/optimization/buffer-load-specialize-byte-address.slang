//TEST:SIMPLE(filecheck=SPV): -target spirv -O0 -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=IR0): -target spirv-asm -O0 -dump-ir-before legalizeByteAddressBufferOps
//TEST:SIMPLE(filecheck=IR8): -target spirv-asm -O0 -dump-ir-before legalizeByteAddressBufferOps
//TEST:SIMPLE(filecheck=IR16): -target spirv-asm -O0 -dump-ir-before legalizeByteAddressBufferOps

// Buffer-load argument specialization must preserve the literal alignment of each load.
// We pass a large `Frame` to `makeRay` to trigger that specialization, then inspect the
// reconstructed loads before byte-address legalization replaces them with storage-buffer loads.
// The two calls with alignment 8 use different runtime offsets and must share a specialization.
// Alignment 0 from `Load` and the explicit alignment 16 must select different specializations.

// The two matrices make `Frame` large enough for buffer-load argument specialization.
struct Frame
{
    float4x4 unusedMatrix;
    float4x4 transform;
    float3 positionOffset;
    float padding;
};

float3 makeRay(Frame frame)
{
    float4 position = mul(float4(0, 0, 0, 1), frame.transform);
    return position.xyz + frame.positionOffset;
}

uniform ByteAddressBuffer buf;
uniform RWTexture2D<float4> outTexture;
uniform uint width;
uniform uint height;

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    if (dtid.x >= width || dtid.y >= height) return;
    uint offset = dtid.z * 1024;
    Frame defaultAligned = buf.Load<Frame>(offset + 256);
    Frame aligned8 = buf.LoadAligned<Frame>(offset, 8);
    Frame aligned16 = buf.LoadAligned<Frame>(offset + 128, 16);
    Frame anotherAligned8 = buf.LoadAligned<Frame>(offset + 512, 8);
    outTexture[dtid.xy] = float4(
        makeRay(defaultAligned) + makeRay(aligned8) + makeRay(aligned16) + makeRay(anotherAligned8),
        1);
}

// SPV: OpEntryPoint GLCompute

// Each prefix identifies a callee from `computeMain` and checks the load inside that helper.
// Its offset must come from the helper parameter, while its alignment remains a literal.
// Separate prefixes allow the helper definitions to appear in any order.
// IR0: ### BEFORE legalizeByteAddressBufferOps:
// IR0: func %computeMain
// IR0-NOT: {{^}}}
// IR0: call [[ALIGN0:%makeRay[^ (]*]](
// IR0: func [[ALIGN0]] : Func(Vec(Float, 3 : Int), UInt)
// IR0-NOT: {{^}}}
// IR0: param [[OFFSET0:%[0-9]+]] : UInt
// IR0-NOT: {{^}}}
// IR0: byteAddressBufferLoad(%buf, [[OFFSET0]], 0 : UInt)

// The second and fourth calls must share the alignment-8 specialization despite their offsets.
// IR8: ### BEFORE legalizeByteAddressBufferOps:
// IR8: func %computeMain
// IR8-NOT: {{^}}}
// IR8: call %makeRay{{[^ (]*}}(
// IR8-NOT: {{^}}}
// IR8: call [[ALIGN8:%makeRay[^ (]*]](
// IR8-NOT: {{^}}}
// IR8: call %makeRay{{[^ (]*}}(
// IR8-NOT: {{^}}}
// IR8: call [[ALIGN8]](
// IR8: func [[ALIGN8]] : Func(Vec(Float, 3 : Int), UInt)
// IR8-NOT: {{^}}}
// IR8: param [[OFFSET8:%[0-9]+]] : UInt
// IR8-NOT: {{^}}}
// IR8: byteAddressBufferLoad(%buf, [[OFFSET8]], 8 : UInt)

// IR16: ### BEFORE legalizeByteAddressBufferOps:
// IR16: func %computeMain
// IR16-NOT: {{^}}}
// IR16: call %makeRay{{[^ (]*}}(
// IR16-NOT: {{^}}}
// IR16: call %makeRay{{[^ (]*}}(
// IR16-NOT: {{^}}}
// IR16: call [[ALIGN16:%makeRay[^ (]*]](
// IR16: func [[ALIGN16]] : Func(Vec(Float, 3 : Int), UInt)
// IR16-NOT: {{^}}}
// IR16: param [[OFFSET16:%[0-9]+]] : UInt
// IR16-NOT: {{^}}}
// IR16: byteAddressBufferLoad(%buf, [[OFFSET16]], 16 : UInt)
