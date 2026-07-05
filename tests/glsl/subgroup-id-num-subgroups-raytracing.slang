// Negative-path test for #11303: the GLSL-compatibility builtins gl_SubgroupID
// and gl_NumSubgroups (in glsl.meta.slang) wrap WaveGetWaveIndex() /
// WaveGetNumWaves() and carry the same subgroup_workgroup_index capability.
// Reading them from a raytracing stage must be rejected at compile time with a
// capability error rather than emitting BuiltIn SubgroupId / NumSubgroups, which
// the Vulkan SPIR-V environment spec restricts to compute-class execution models
// (VUID-SubgroupId-SubgroupId-04367 / VUID-NumSubgroups-04293).
//
// The capability is declared on the property declarations themselves (not only
// their getters) so that a property read, which resolves to the PropertyDecl,
// propagates the requirement to the entry point.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target spirv -entry main -stage raygeneration -emit-spirv-directly -allow-glsl

RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
/*CHECK:
     ^^^^ E36107
     ^^^^ uses features that are not available in '_raygen' stage
*/
{
    output[0] = gl_SubgroupID;
    output[1] = gl_NumSubgroups;
}
