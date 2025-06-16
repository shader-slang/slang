//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target spirv -emit-spirv-directly -entry main -stage compute -ignore-capabilities
// CHECK_IGNORE_CAPS-NOT: error 36108

// Check that a non-static member method implictly requires capabilities
// defined in ThisType.

// CHECK: error 36119:{{.*}}glsl
[require(hlsl)]
struct Type
{
    int member;

    [require(glsl)]
    static void f1()
    {
    }
}

void main()
{}
