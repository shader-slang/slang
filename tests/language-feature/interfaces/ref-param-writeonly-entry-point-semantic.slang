//DIAGNOSTIC_TEST:SIMPLE(diag=GROUP):-target spirv -entry groupIndex -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=STENCIL):-target spirv -entry stencil -stage fragment -no-codegen

// A `__ref_writeonly` entry-point parameter is an output of the stage, like an `out` parameter,
// so its system-value semantic is validated in the output direction only. Both tests are
// exhaustive: `SV_StencilRef`, an output-only semantic, is accepted with no diagnostic.

[numthreads(1, 1, 1)]
void groupIndex(__ref_writeonly uint index : SV_GroupIndex)
//GROUP:                             ^^^^^ system value semantic 'SV_GroupIndex' cannot be used as output in 'compute' shader stage
{
    index = 1;
}

void stencil(__ref_writeonly uint value : SV_StencilRef)
{
    value = 1;
}
