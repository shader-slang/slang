//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -entry main -stage compute

// Test that capabilities can be declared on module.

[require(glsl)]
[require(spirv)]
module test;

void f()
{
    __require_capability glsl;
}

// CHECK: ([[# @LINE+1]]): error 36108
public void g()
{
    __require_capability spvAtomicFloat16AddEXT;
}

void l()
{
    __target_switch
    {
    case glsl:
        f();
        return;
    case spirv:
        __require_capability spvAtomicFloat16AddEXT;
        return;
    }
}

// CHECK: ([[# @LINE+1]]): error 36104: {{.*}}
public void use()
{
    l(); // Error
}

// CHECK-NOT: ([[# @LINE+1]]): error
[require(spirv, spvAtomicFloat16AddEXT)]
public void use1()
{
    l(); // Error
}

void main()
{}