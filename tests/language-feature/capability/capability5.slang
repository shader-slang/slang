//TEST:SIMPLE(filecheck=PASS): -target glsl -entry main -stage compute -allow-glsl
//TEST:SIMPLE(filecheck=PASS): -target glsl -entry main -stage compute -allow-glsl -capability sm_4_0
//TEST:SIMPLE(filecheck=WARN): -target glsl -entry main -stage compute -allow-glsl -capability GLSL_130
//TEST:SIMPLE(filecheck=ERROR): -target glsl -entry main -stage compute -allow-glsl -capability GLSL_130 -restrictive-capability-check
// CHECK_IGNORE_CAPS-NOT: error 36104

// Check that a non-static member method implictly requires capabilities
// defined in ThisType.

//PASS-NOT: warning 41012
//PASS-NOT: error 41012

//WARN: warning 41012
//WARN-NOT: error 41012

//ERROR-NOT: warning 41012
//ERROR: error 41012

buffer MyBlockName
{
    float data;
} inputBuffer;

[numthreads(1,1,1)]
void main()
{
    inputBuffer.data = exp(2.0f);
}
