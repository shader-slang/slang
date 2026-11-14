//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A declaration may have the module's name, so the module's own `module` declaration does not
// keep a later declaration from being compared with a buffer member of that name
// (shader-slang/slang#13876). The `module` declaration must come first, so this case has its
// own file.

module lighting;

cbuffer LightingBuffer
{
    float3 lighting;
/*CHECK:
           ^^^^^^^^ see previous declaration of 'lighting'
*/
}

static const float3 lighting = float3(1, 1, 1);
/*CHECK:
                    ^^^^^^^^ warning E30204
                    ^^^^^^^^ declaration of 'lighting' conflicts with existing declaration
*/
