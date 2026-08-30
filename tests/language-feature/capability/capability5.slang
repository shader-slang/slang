//TEST:SIMPLE_EX(filecheck=PASS): -lang glsl tests/language-feature/capability/capability5.slang -target glsl -entry main -stage compute -Wno-118
//TEST:SIMPLE_EX(filecheck=PASS): -lang glsl tests/language-feature/capability/capability5.slang -target glsl -entry main -stage compute -capability shader5_sm_5_0 -Wno-118
//TEST:SIMPLE_EX(filecheck=PASS): -lang glsl tests/language-feature/capability/capability5.slang -target glsl -entry main -stage compute -profile sm_6_0 -Wno-118
//TEST:SIMPLE_EX(filecheck=WARN): -lang glsl tests/language-feature/capability/capability5.slang -target glsl -entry main -stage compute -capability GLSL_130 -Wno-118
//TEST:SIMPLE_EX(filecheck=ERROR): -lang glsl tests/language-feature/capability/capability5.slang -target glsl -entry main -stage compute -capability GLSL_130 -restrictive-capability-check -Wno-118
// CHECK_IGNORE_CAPS-NOT: E41012

// Check that a non-static member method implictly requires capabilities
// defined in ThisType.

//PASS-NOT: warning[E41012]
//PASS-NOT: error[E41012]

//WARN: warning[E41012]
//WARN-NOT: error[E41012]

//ERROR-NOT: warning[E41012]
//ERROR: error[E41013]

buffer MyBlockName
{
    int data;
} inputBuffer;

[numthreads(1,1,1)]
void main()
{
    inputBuffer.data = firstbithigh(1);
}
