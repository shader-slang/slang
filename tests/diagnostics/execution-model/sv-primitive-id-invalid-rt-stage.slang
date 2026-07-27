// Diagnostic: `SV_PrimitiveID` is readable in the intersection, any-hit,
// and closest-hit stages (see tests/vkray/intersection-primitive-id.slang),
// but there is no current primitive in the raygeneration and miss stages,
// so it must still be rejected there.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry missMain

//CHECK: E30702
[shader("miss")]
void missMain(inout float3 payload, uint primitiveId: SV_PrimitiveID)
{
    payload.x = primitiveId;
}
