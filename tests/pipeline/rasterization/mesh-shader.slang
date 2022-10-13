// mesh-shader.slang

// Test that a simple mesh shader compiles

//TEST:CROSS_COMPILE:-target glsl -profile glsl_450+spirv_1_4 -entry main -stage mesh

[shader("mesh")]
void main()
{
}
