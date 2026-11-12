//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry fragMain -stage fragment

// A `RasterizerOrderedByteAddressBuffer` that is reached only through `GetDimensions` and
// `InterlockedAdd` produces `GetEquivalentStructuredBuffer` and no load or store, so the
// buffer's type is what makes byte-address buffer legalization run for SPIR-V.

RasterizerOrderedByteAddressBuffer buf;

// CHECK: OpDecorate %buf Binding 0
// CHECK: %buf = OpVariable
// CHECK: OpArrayLength
// CHECK: OpAtomicIAdd

float4 fragMain() : SV_Target
{
    uint dim;
    buf.GetDimensions(dim);
    uint orig;
    buf.InterlockedAdd(0, 1, orig);
    return float4(dim, orig, 0, 1);
}
