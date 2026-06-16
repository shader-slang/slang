//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_3

// Regression test for #11631.
//
// An entry-point `[require(spirv_1_x)]` attribute must raise the emitted SPIR-V
// binary version the same way `-capability spirv_1_x` / `-profile spirv_1_x`
// does. `[require(spirv_1_5)]` lowers to an IRRequireCapabilityAtomDecoration
// carrying the internal `_spirv_1_5` atom; determineSpirvVersion() previously
// scanned that decoration but matched only the public `spirv_1_5` alias, so the
// requirement was silently dropped and the version stayed at the profile floor.
//
// Baseline profile is 1.3; the [require(spirv_1_5)] must bump the header to 1.5.

// CHECK: Version: 1.5

[require(spirv_1_5)]
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}
