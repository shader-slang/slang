//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -emit-spirv-directly -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_IGNORE_CAPS): -target spirv -emit-spirv-directly -entry main -stage compute -ignore-capabilities

// Check that a non-static member method implictly requires capabilities
// defined in ThisType.

[require(hlsl)]
struct Type
/*CHECK:
       ^^^^ see definition of 'Type'
*/
{
    int member;
    [require(glsl)]
    [mutating]
    void f()
//CHECK: ^ dependencies not compatible on target
//CHECK: ^ 'f' has dependencies that are not compatible on the required compilation target 'glsl'.
    {
    }

    [require(glsl)]
    static void f1()
    {
    }
}

void main()
{}
