//TEST_IGNORE_FILE:

// Helper for vertex-missing-sv-position-global.slang. It imports the `glsl` module, so a module that
// imports this helper depends on the `glsl` module indirectly.
import glsl;

float4 helperColor()
{
    return float4(1);
}
