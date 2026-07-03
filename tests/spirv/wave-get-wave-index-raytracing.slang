// Verify that WaveGetWaveIndex() and WaveGetNumWaves() are rejected at compile
// time when used from a raytracing stage targeting SPIR-V or GLSL.
//
// Both lower to GLSL gl_SubgroupID / gl_NumSubgroups and SPIR-V BuiltIn
// SubgroupId / NumSubgroups, which the GLSL and Vulkan SPIR-V environment specs
// restrict to compute-class execution models (compute/mesh/task). Using them
// from a raygeneration entry point previously emitted SPIR-V that fails
// spirv-val with VUID-SubgroupId-SubgroupId-04367 (and the analogous
// NumSubgroups VUID). The `subgroup_workgroup_index` capability now encodes that
// stage restriction so the misuse is caught by the capability system at compile
// time on both targets. See issue #11303.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target spirv -entry main -stage raygeneration -emit-spirv-directly
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target glsl -entry main -stage raygeneration

RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
/*CHECK:
     ^^^^ E36107
     ^^^^ uses features that are not available in '_raygen' stage
*/
{
    output[0] = WaveGetWaveIndex();
    output[1] = WaveGetNumWaves();
}
