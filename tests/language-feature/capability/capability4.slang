//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -entry main -stage compute

// Check that a non-static member method implictly requires capabilities
// defined in ThisType.

[require(hlsl)]
struct Type
{
    int member;
    [require(glsl)]
    [mutating]
    // CHECK: ([[# @LINE+1]]): error 36108:
    void f()
    {
    }

    [require(glsl)]
    // CHECK-NOT: ([[# @LINE+1]]): error 36108:
    static void f1()
    {
    }
}

void main()
{}
