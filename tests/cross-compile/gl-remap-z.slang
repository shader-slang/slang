// gl-remap-z.slang

// Test that '-fgl-remap-z' remaps SV_Position.z from the OpenGL clip-space depth range
// [-w, w] to the standard [0, w] range (z' = (z + w) / 2) on the GLSL vertex output, and
// that it composes with '-fvk-invert-y' (the two transforms touch independent components).

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage vertex -profile vs_5_1 -fgl-remap-z
//TEST:SIMPLE(filecheck=COMPOSE):-target glsl -entry main -stage vertex -profile vs_5_1 -fvk-invert-y -fgl-remap-z
//TEST:SIMPLE(filecheck=NOREMAP):-target glsl -entry main -stage vertex -profile vs_5_1

struct VOutput
{
    float4 v : SV_Position;
}


VOutput main()
{
    VOutput output;
    output.v = float4(1, 2, 3, 4);

    // The z component of the written position is set from an expression that reads both its
    // z and its w (i.e. z' = (z + w) / 2), then the result is written to gl_Position.
    // CHECK: [[TMP:[_A-Za-z0-9]+]].z = {{.*}}output_0.v_0.z{{.*}}output_0.v_0.w{{.*}};
    // CHECK: gl_Position = [[TMP]];

    // Composed with -fvk-invert-y: y is additively inverted AND z is remapped reading both
    // z and w. The two transforms are independent, so order does not matter (CHECK-DAG).
    // COMPOSE-DAG: {{[_A-Za-z0-9]+}}.y = - output_0.v_0.y;
    // COMPOSE-DAG: {{[_A-Za-z0-9]+}}.z = {{.*}}.z{{.*}}.w{{.*}};
    // COMPOSE: gl_Position =

    // Off by default: without '-fgl-remap-z' the position is written unmodified, with no
    // (z + w) / 2 temp, confirming the remap is a pure opt-in (and that the gate, not the
    // shader, controls it).
    // NOREMAP-NOT: / 2.0
    // NOREMAP: gl_Position = output_0.v_0;
    return output;
}
