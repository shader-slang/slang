//TEST:SIMPLE_EX(filecheck=CHECK_GLSL): -lang glsl tests/glsl/ssboStructInsideStructMemoryQualifierError.slang -stage compute -entry computeMain -target glsl -Wno-118
//TEST:SIMPLE_EX(filecheck=CHECK_SPV): -lang glsl tests/glsl/ssboStructInsideStructMemoryQualifierError.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly -Wno-118

// This code should error since memory qualifiers are only allowed inside:
// Shader storage blocks, variables declared within shader storage blocks 
// and variables declared as image types. Named structs inside a Interface
// block violates these rules
// CHECK_GLSL: error
// CHECK_SPV: error
buffer Block4 {
    struct StructTmp
    {
        readonly int val;
    };
    readonly structTmp myStruct;
    coherent readonly vec4 member1;
    coherent vec4 member2;
}inputBuffer4;

layout(local_size_x = 1) in;
void computeMain()
{
}