//TEST:SIMPLE(filecheck=CHK): -target glsl -stage fragment -entry main -allow-glsl

// Test for GLSL mode: global const variables with initializers should be allowed
// In GLSL mode, global const variables are real constants, not uniform parameters
// This should NOT produce the error 31224 that would trigger in HLSL mode

#version 450

// These should NOT trigger error 31224 in GLSL mode (they would in HLSL)
const float globalConstWithInit = 1.0;        // OK in GLSL - real constant
const vec3 globalVecConst = vec3(1.0, 2.0, 3.0); // OK in GLSL - real constant
const int globalIntConst = 42;                // OK in GLSL - real constant

// Regular uniforms without const should still be allowed
uniform float uniformFloat;                   // OK - uniform without const
uniform vec4 uniformVec;                     // OK - uniform without const

// CHK-NOT: error 31224
// CHK: void main()

out vec4 fragColor;

void main() 
{
    fragColor = vec4(globalConstWithInit, globalVecConst.x, globalIntConst, 1.0);
}