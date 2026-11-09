// Companion module for `precompiled-entry-without-shader-attribute.slang`. `vmain` has no
// `[shader]` attribute; it becomes an entry point only because the command line that compiles
// this module requests it with `-entry vmain -stage vertex`.
module vertex_module;

public float4 vmain(float4 p: POSITION) : SV_Position
{
    return p * 2;
}
