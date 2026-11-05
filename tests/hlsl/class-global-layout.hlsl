//TEST:SIMPLE(filecheck=CHECK): -target spirv -no-codegen
// CHECK: result code = 0

// HLSL classes are value types and can be laid out as global shader parameters.
// https://github.com/shader-slang/slang/issues/10305
class X
{
    int a;
};

uniform X x;
