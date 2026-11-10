//TEST_IGNORE_FILE:

// Helper for vertex-missing-sv-position-global.slang. It imports the `glsl` module, so a module that
// imports this helper depends on the `glsl` module indirectly, and its entry point can write
// `gl_Position` by calling `helperSetPosition`.
import glsl;

void helperSetPosition()
{
    gl_Position = float4(0, 0, 0, 1);
}
